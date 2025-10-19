#include "Lexer/lexer.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <map>
#include <set>
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
void Lexer::GenMarch(bool (*func)(char), TokenKind tok,
                     bool shouldSaveLiteral) {
  char curChar = GetCurrChar();
  while (func(curChar) && !bCompletedLex) {
    ChunkBuffer.push_back(curChar);
    curChar = March();
  }
}
void Lexer::MarchWord() {
  auto func = [](char c) -> bool { return isalnum(c); };
  GenMarch(func, TOK_TYPES::WORD, true);
  // TODO: We have to determine if this word is a keyword or an identifier
  if (/*Use lookup to determine if this is a keyword*/) {

  } else {
    // identifier
    TokenBuffer.push_back(Token(
        TokenKind::identifier, string(ChunkBuffer.begin(), ChunkBuffer.end())));
  }
  ChunkBuffer.clear();
}

void Lexer::MarchNum() {
  auto func = [](char c) -> bool { return isdigit(c); };
  GenMarch(func, TOK_TYPES::DIGIT, true);
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
