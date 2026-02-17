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
#include <fstream>
#include <sstream>
using namespace std;
using namespace Brain;
using namespace tok;

Lexer::Lexer() {
  PopulateSymbolTree();
}

Lexer::Lexer(const std::vector<std::string>& files) {
  PopulateSymbolTree();

  const int lexChunkSize = 4;
  char buffer[lexChunkSize];

  for (auto& file : files) {
    std::ifstream ofile(file, std::ios::binary);

    if (!ofile.is_open()) {
      BrainFreeze::ErrorContext context(__FILE__, "", __LINE__, 0);
      BrainFreeze::RuntimeError newError("Unable to open file: " + file, context);
      ErrorBuffer.push_back(newError);
      continue;
    }


    //Lex in chunks. This is so we can handle arbitrary sized files
    while (ofile.read(buffer, sizeof(buffer)) || ofile.gcount() > 0) {
      Lex(&buffer[0], &buffer[ofile.gcount()-1], file,"TODO");
    }

    // for (auto& token : summary.Tokens) {
    //   cout<< token.DEBUG_TOKEN()<<endl;
    // }
    // for (auto& error : summary.Errors) {
    //   cout <<error<<endl;
    // }
  }
}

bool Lexer::issymbol(char c) {
  return (33 <= c && c <= 47) ||
         (58 <= c && c <= 64) ||
         (91 <= c && c <= 96) ||
         (123 <= c && c <= 126); }

void Lexer::CreateToken(tok::TokenKind tokKind, bool bSaveLiteral) {
  if (bSaveLiteral) {
    TokenBuffer.emplace_back(tokKind,
                             string(ChunkBuffer.begin(), ChunkBuffer.end()));
  } else {
    TokenBuffer.emplace_back(tokKind);
  }
  ChunkBuffer.clear();
}
MarchResult Lexer::March(char &c, bool saveToChunkBuffer) {
  // Save current char to buffer
  if (saveToChunkBuffer) {
    ChunkBuffer.push_back(*Head);
  }

  CurrentColumn++;
  if (++Head < End) {
    c = *Head;
    return MarchResult::cont;
  }

  return bLastLexChunk ? MarchResult::eof :  MarchResult::eoc;
}

std::string_view Lexer::ChunkBufferToStringView() {
  return {ChunkBuffer.data(), ChunkBuffer.size()};
}

void Lexer::PopulateSymbolTree() {
  // Populate the symbol trie tree.
#define PUNCTUATOR(NAME, VALUE) TrieTree.insert(VALUE);
#include "Token/tokenkinds.def"
#undef PUNCTUATOR
}

char Lexer::GetCurrChar() const { return *Head; }

MarchResult Lexer::GenMarch(bool (*func)(char), bool saveToChunkBuffer) {
  char curChar = GetCurrChar();
  MarchResult marchResult;
  while (func(curChar)) {
    if (marchResult = March(curChar, saveToChunkBuffer); marchResult > MarchResult::cont) {
      return marchResult;
    }
  }
  return MarchResult::stop;
}
void Lexer::MarchWord() {
  auto func = [](char c) -> bool { return isalnum(c); };

  if (GenMarch(func) == MarchResult::eoc) {
    //TODO: set the last known type?
    return;
  }

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

    switch (March(curChar)) {
      case MarchResult::eoc:
        return;
      case MarchResult::eof:
        CreateToken(tok::TokenKind::numeric_constant, true);
        return;
      default:
        break;
    }
  }
}

void Lexer::MarchSymbols(int32& lexChunkBackTrackAmount) {
  /*Marching symbols is a bit more complicated that others because some symbols
   *can be a combo of punctuators. For e.g = is a token but so is == . So we
   *have to take that into account. We do so with backtracking, which no other march function
   *does.
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
      BrainFreeze::SyntaxError newError("Symbols dont make up an identifier: " + std::string(ChunkBuffer.begin(), ChunkBuffer.end()),
          BrainFreeze::ErrorContext(TargetFileName, TargetFilePath, CurrentLine + 1,
                                    CurrentColumn - ChunkBuffer.size()));

      ErrorBuffer.push_back(newError);

      ChunkBuffer.clear();
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
        Head -= iterSinceLastSeenKeyword + 1;
        CurrentColumn -= iterSinceLastSeenKeyword + 1;
        ChunkBuffer.erase(ChunkBuffer.end() - iterSinceLastSeenKeyword,
                          ChunkBuffer.end());
        makeSymbolToken();
      } else {
        // We never had a valid identifier, clear buffer and exit
        BrainFreeze::SyntaxError newError("Invalid symbol: " + std::string(ChunkBuffer.begin(), ChunkBuffer.end()),
          BrainFreeze::ErrorContext(TargetFileName, TargetFilePath, CurrentLine + 1,
                                    CurrentColumn - ChunkBuffer.size()));

        ErrorBuffer.push_back(newError);

        ChunkBuffer.clear();
      }
      return;
    }

    numOfMarches++;

    switch (March(curChar)) {
      case MarchResult::stop:
      case MarchResult::eof:
        makeSymbolToken();
        return;
      case MarchResult::eoc:
        lexChunkBackTrackAmount = numOfMarches;
        return;
      default:
        break;
    }
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
    if (March(curChar, false) > MarchResult::cont) {
      break;
    }
  }
}
void Lexer::MarchCommentLine() {
  auto func = [](char c) -> bool { return c != '\n'; };
  if (GenMarch(func) == MarchResult::eoc) {
    return;
  }
  NewLine();
  // WARNING: If we ever change this to where we don't call create token, be
  // sure to clear the chunk buffer
  CreateToken(tok::comment, false);
}

void Lexer::MarchString() {
  auto func = [](char c) -> bool { return c == '"'; };
  if (GenMarch(func) == MarchResult::eoc) {
    return;
  }
  CreateToken(tok::string_literal, true);
}

int32 Lexer::Lex(const char *start, const char *end,
                              const std::string &targetFileName,
                              const std::string &targetFilePath,
                              bool bIsLastLexChunk) {

  // Re/set state
  Start = start;
  End = end;

  TargetFileName = targetFileName;
  TargetFilePath = targetFilePath;
  bLastLexChunk = bIsLastLexChunk;

  while (!IsLexComplete()) {
    char curChar = GetCurrChar();
    if (isalpha(curChar)) {
      MarchWord();
    } else if (isdigit(curChar)) {
      MarchNum();
    } else if (isblank(curChar) || curChar == '\n') {
      MarchBlank();
    } else if (issymbol(curChar)) {
      int32 backtrackLexChunkAmount = 0;
      MarchSymbols(backtrackLexChunkAmount);
      if (backtrackLexChunkAmount != 0) {
        return backtrackLexChunkAmount;
      }
    } else {
      throw std::logic_error("Unhandled lex case");
    }
  }

  return 0;
}
