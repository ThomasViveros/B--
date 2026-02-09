#include "Lexer/lexer.h"
#include "Error/BrainFreeze/ErrorTypes.h"
#include "Lexer/IdentifierTable.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace std;
using namespace Brain;
using namespace tok;

Lexer::Lexer() {
  // Populate the trie tree.
#define PUNCTUATOR(NAME, VALUE) TrieTree.insert(VALUE);
#include "Token/tokenkinds.def"
#undef PUNCTUATOR
}

bool Lexer::issymbol(char c) { return !(isalnum(c) || isspace(c)); }

void Lexer::CreateToken(tok::TokenKind tokKind, bool bSaveLiteral) {
  if (bSaveLiteral) {
    TokenBuffer.emplace_back(tokKind,
                             string(ChunkBuffer.begin(), ChunkBuffer.end()));
  } else {
    TokenBuffer.emplace_back(tokKind);
  }
  ChunkBuffer.clear();
}
bool Lexer::March(char &c, bool saveToChunkBuffer) {
  // Save current char to buffer
  if (saveToChunkBuffer) {
    ChunkBuffer.push_back(Start[HeadIndx]);
  }

  if (Start + ++HeadIndx < End) {
    c = Start[HeadIndx];
    CurrentColumn++;
    return false;
  }
  bCompletedLex = true;
  return true;
}

std::string_view Lexer::ChunkBufferToStringView() {
  return {ChunkBuffer.data(), ChunkBuffer.size()};
}
char Lexer::GetCurrChar() const { return Start[HeadIndx]; }

void Lexer::GenMarch(bool (*func)(char), bool saveToChunkBuffer) {
  char curChar = GetCurrChar();
  while (func(curChar)) {
    if (March(curChar, saveToChunkBuffer)) {
      break;
    }
  }
}
void Lexer::MarchWord() {
  auto func = [](char c) -> bool { return isalnum(c); };

  GenMarch(func);

  TokenKind keywordToken = unknown;
  if (IdentifierTable.IsIdentifier(ChunkBufferToStringView(),
                                   /*OUT*/ keywordToken)) {
    CreateToken(keywordToken, false);
  } else {
    // identifier
    CreateToken(tok::identifier, true);
  }
}

void Lexer::MarchNum() {
  // Number may be int or float, accept a single period and determine actual
  // literal type during semantic analysis.

  char curChar = GetCurrChar();
  bool bHasPeriodBeenFound = false;
  bool bIsPeriod = false;

  while (isdigit(curChar) || (bIsPeriod && !bHasPeriodBeenFound)) {
    bIsPeriod = curChar == '.';
    if (bIsPeriod) {
      bHasPeriodBeenFound = true;
    }

    if (March(curChar)) {
      break;
    }
  }

  CreateToken(tok::TokenKind::numeric_constant, true);
}

void Lexer::MarchSymbols() {
  /*Marching symbols is a bit more complicated that others because some symbols
   *can be a combo of punctuators. For e.g = is a token but so is == . So we
   *have to take that into account
   *
   *Also, the other march functions will try to completely resolve their string,
   *but in this func it is possible to leave even if there are symbols left.
   *This is too simplify the function. Our main loop will handle this and call
   *this function again if that is the case
   *
   *Regardless our rule still applies that we will leave the function with the
   *chunk buffer empty and the head at the next symbol to march.
   **/
  char curChar = GetCurrChar();

  TrieNode *curNode = TrieTree.GetRoot();
  int32 iterSinceLastSeenKeyword = 0;
  int32 numOfMarches = 0;

  auto makeSymbolToken = [this]() {
    tok::TokenKind tokenKind;
    if (IdentifierTable.IsIdentifier(ChunkBufferToStringView(),
                                     /*OUT*/ tokenKind)) {
      // If this is a comment, dont waste time trying to tokenize, just
      // determine the end of the comment and discard.
      if (tokenKind == tok::TokenKind::slashslash) {
        // We detected a comment, discard these symbols and start marching the
        // comment line
        MarchCommentLine();
      } else {
        CreateToken(tokenKind, false);
        // dummy char
        char dummy;
        March(dummy, false);
      }
    } else {
      // Throw error, we have a combination of symbols that dont make a keyword
      ErrorBuffer.emplace_back(
          "Symbols dont make up an identifier",
          BrainFreeze::ErrorContext(TargetFileName, TargetFilePath, CurrentLine,
                                    CurrentColumn - ChunkBuffer.size()));
    }
  };

  while (issymbol(curChar)) {
    curNode = curNode->GetChild(curChar);

    if (curNode) {
      if (curNode->children.empty()) {
        // There is no more possible moves we could do, attempt to make token
        if (curNode->bIsIdentifier) {
          // Make token, we are done
          ChunkBuffer.push_back(curChar);
          makeSymbolToken();
          return;
        } else {
          throw std::logic_error("Symbol was the last element in trie chain "
                                 "yet it isnt marked identifier!!!");
        }
      } else {
        if (curNode->bIsIdentifier) {
          iterSinceLastSeenKeyword = 0;
        } else {
          iterSinceLastSeenKeyword++;
        }
      }
    } else {
      // Try to backtrack
      // First check if an identifier was ever detected
      if (iterSinceLastSeenKeyword != numOfMarches) {
        HeadIndx -= iterSinceLastSeenKeyword + 1;
        CurrentColumn -= iterSinceLastSeenKeyword + 1;
        ChunkBuffer.erase(ChunkBuffer.end() - iterSinceLastSeenKeyword,
                          ChunkBuffer.end());
        bCompletedLex = false;
        makeSymbolToken();
      } else {
        // We never had a valid identifier, clear buffer and exit
        ErrorBuffer.emplace_back(
            "Invalid symbol", BrainFreeze::ErrorContext(
                                  TargetFileName, TargetFilePath, CurrentLine,
                                  CurrentColumn - ChunkBuffer.size()));

        ChunkBuffer.clear();
      }
      return;
    }

    if (March(curChar)) {
      makeSymbolToken();
      return;
    }
    numOfMarches++;
  }

  ChunkBuffer.clear();
}
void Lexer::MarchBlank() {

  auto func = [](char c) -> bool { return isblank(c) || c == '\n'; };

  char curChar = GetCurrChar();
  while (func(curChar)) {
    if (curChar == '\n') {
      NewLine();
    }
    if (March(curChar, false)) {
      break;
    }
  }
}
void Lexer::MarchCommentLine() {
  auto func = [](char c) -> bool { return c != '\n'; };
  GenMarch(func);
  NewLine();
  // WARNING: If we ever change this to where we don't call create token, be
  // sure to clear the chunk buffer
  CreateToken(tok::comment, false);
}

void Lexer::MarchString() {
  auto func = [](char c) -> bool { return c == '"'; };
  GenMarch(func);
  CreateToken(tok::string_literal, true);
}

std::vector<Token> Lexer::Lex(const char *start, const char *end,
                              const std::string &targetFileName,
                              const std::string &targetFilePath) {

  // Reset state
  Start = start;
  End = end;

  if (start >= end) {
    return {};
  }

  TargetFileName = targetFileName;
  TargetFilePath = targetFilePath;

  while (bCompletedLex != true) {
    char curChar = GetCurrChar();
    if (isalpha(curChar)) {
      MarchWord();
    } else if (isdigit(curChar)) {
      MarchNum();
    } else if (isblank(curChar) || curChar == '\n') {
      MarchBlank();
    } else if (issymbol(curChar)) {
      MarchSymbols();
    } else {
      throw std::logic_error("Unhandled lex case");
    }
  }
  return TokenBuffer;
}
