#include "Lexer/lexer.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include "Lexer/IdentifierTable.h"
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

using namespace std;
using namespace Brain;
using namespace tok;

bool Lexer::issymbol(char c) { return !(isalnum(c) || isspace(c)); }

char Lexer::March() {
  if (start + ++HeadIndx <= end) {
    return start[HeadIndx];
  }
  bCompletedLex = true;
  return NULL;
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
  if (IdentifierTable::GetIdentifierTable().IsKeyword(std::string_view(ChunkBuffer.data(),ChunkBuffer.size()),keywordToken)) {
    TokenBuffer.push_back(Token(keywordToken));
  } else {
    // identifier
    TokenBuffer.push_back(Token(
        TokenKind::identifier, string(ChunkBuffer.begin(), ChunkBuffer.end())));
  }
  ChunkBuffer.clear();
}

void Lexer::MarchNum() {
  //Number may be int or float, accept a single period and determine actual literal type during
  //semantic analysis.

  char curChar = GetCurrChar();
  bool bHasPeriodBeenFound = false;
  bool bIsPeriod = false;

  while (isdigit(curChar) || (bIsPeriod && !bHasPeriodBeenFound) && !bCompletedLex) {
    bIsPeriod = curChar == '.';

    ChunkBuffer.push_back(curChar);
    curChar = March();

    if (bIsPeriod) {
      bHasPeriodBeenFound = true;
    }
  }

  TokenBuffer.push_back(Token(TokenKind::numeric_constant, string(ChunkBuffer.begin(), ChunkBuffer.end())));

  ChunkBuffer.clear();
}

void Lexer::MarchSymbols() {

  char curChar = GetCurrChar();
  while (issymbol(curChar)) {
    ChunkBuffer.push_back(curChar);
    int count = TrieTree.prefixCount(ChunkBuffer.begin(), ChunkBuffer.end());
    if (count == 1) {
      TokenBuffer.push_back(
          Token::DetOpTok(string_view(ChunkBuffer.data(), ChunkBuffer.size())));
      break;
    } else if (count == 0) {
      // Invalid syntax, TODO: DEBUGGER
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
void Lexer::MarchCommentLine() {}

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
