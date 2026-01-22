#include "Lexer/lexer.h"
#include "Lexer/IdentifierTable.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

using namespace std;
using namespace Brain;
using namespace tok;

bool Lexer::issymbol(char c) { return !(isalnum(c) || isspace(c)); }

void Lexer::CreateToken(tok::TokenKind tokKind, bool bSaveLiteral) {
  if (bSaveLiteral) {
    TokenBuffer.push_back(
        Token(tokKind, string(ChunkBuffer.begin(), ChunkBuffer.end())));
  } else {
    TokenBuffer.push_back(Token(tokKind));
  }
  ChunkBuffer.clear();
}
char Lexer::March() {
  if (start + ++HeadIndx <= end) {
    return start[HeadIndx];
  }
  bCompletedLex = true;
  return NULL;
}

void Lexer::MarchBack(int32 amount) {
  ChunkBuffer.erase(ChunkBuffer.end() - amount, ChunkBuffer.end());
  HeadIndx -= amount;
  // bCompletedLex = false;???
}
std::string_view Lexer::ChunkBufferToStringView() {
  return std::string_view(ChunkBuffer.data(), ChunkBuffer.size());
}
char Lexer::GetCurrChar() { return start[HeadIndx]; }

void Lexer::GenMarch(bool (*func)(char)) {
  char curChar = GetCurrChar();
  while (func(curChar) && !bCompletedLex) {
    ChunkBuffer.push_back(curChar);
    curChar = March();
  }
}
void Lexer::MarchWord() {
  auto func = [](char c) -> bool { return isalnum(c); };

  GenMarch(func);

  TokenKind keywordToken = unknown;
  if (IdentifierTable::GetIdentifierTable().IsKeyword(ChunkBufferToStringView(),
                                                      keywordToken)) {
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

  while (isdigit(curChar) ||
         (bIsPeriod && !bHasPeriodBeenFound) && !bCompletedLex) {
    bIsPeriod = curChar == '.';

    ChunkBuffer.push_back(curChar);
    curChar = March();

    if (bIsPeriod) {
      bHasPeriodBeenFound = true;
    }
  }

  CreateToken(tok::TokenKind::numeric_constant, true);
}

void Lexer::MarchSymbols() {

  char curChar = GetCurrChar();

  TrieNode *curNode = TrieTree.GetRoot();
  int32 iterSinceLastSeenKeyword = 0;

  auto lambda = []() {
    tok::TokenKind tokenKind;

    if (IdentifierTable::GetIdentifierTable().IsKeyword(
            ChunkBufferToStringView(), /*OUT*/ tokenKind)) {

      // If this is a comment, dont waste time trying to tokenize, just
      // determine the end of the comment and discard.
      if (tokenKind == tok::TokenKind::comment) {
        // We detected a comment, discard these symbols and start marching the
        // comment line
        ChunkBuffer.clear();
        MarchCommentLine();
        return;
      } else {
        CreateToken(tokenKind, false);
        break;
      }
    }
  };
  while (issymbol(curChar)) {
    ChunkBuffer.push_back(curChar);

    curNode = curNode->GetChild(curChar);
    if (!curNode->bIsIdentifier) {
      iterSinceLastSeenKeyword++;
    }

    //  TODO: we need to do this incrementally, currently were doing
    //  inefficiently.
    // this means that there is no more possible identifiers this could become,
    // make sure this is a token, else we have to backtrack.
    if (curNode->children.size() < 1) {
      if (curNode->bIsIdentifier) {
        // Make the token, do comment check etc
      } else {
        // backtrack
        MarchBack(iterSinceLastSeenKeyword);
        // Make the token
      }
    }
    if (count == 1) {

      tok::TokenKind tokenKind;

      if (IdentifierTable::GetIdentifierTable().IsKeyword(
              ChunkBufferToStringView(), /*OUT*/ tokenKind)) {

        // If this is a comment, dont waste time trying to tokenize, just
        // determine the end of the comment and discard.
        if (tokenKind == tok::TokenKind::comment) {
          // We detected a comment, discard these symbols and start marching the
          // comment line
          ChunkBuffer.clear();
          MarchCommentLine();
          return;
        } else {
          CreateToken(tokenKind, false);
          break;
        }
      }
    }
    curChar = March();
  }

  ChunkBuffer.clear();
}
void Lexer::MarchBlank() {
  char curChar = March();
  while (isblank(curChar)) {
    curChar = March();
  }
}
void Lexer::MarchCommentLine() {
  auto func = [](char c) -> bool { return c == '\n'; };
  GenMarch(func);

  CreateToken(tok::comment, false);
}

void Lexer::MarchString() {
  auto func = [](char c) -> bool { return c == '"'; };
  GenMarch(func);
  CreateToken(tok::string_literal, true);
}

void Lexer::Lex(char *start, char *end) {
  // Reset state
  bCompletedLex = false;
  while (!bCompletedLex) {

    char curChar = GetCurrChar();
    if (isalpha(curChar)) {
      MarchWord();
    } else if (isdigit(curChar)) {
      MarchNum();
    } else if (isblank(curChar)) {
      MarchBlank();
    } else if (issymbol(curChar)) { // TODO: Make sure symbol doesnt include ""
                                    // and //, this is reserved for strings and
                                    // comments respectively
      // Perhaps we could handle this inside of the MarchSymbols func
      MarchSymbols();
    }
  }
}
