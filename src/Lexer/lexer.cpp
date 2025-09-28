#include "Lexer/lexer.h"
#include "Lexer/token.h"
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

const map<string, TOK_TYPES> &GetTokenMap() {
  static map<std::string, TOK_TYPES> TokenMap = {{"func", TOK_TYPES::FUNC},
                                                 {"void", TOK_TYPES::VOID}};
  return TokenMap;
}
/*
auto it_tok =
      GetTokenMap().find(string(ChunkBuffer.begin(), ChunkBuffer.end()));
  // Determine if this word is a keyword
  if (it_tok != GetTokenMap().end()) {
    TokenBuffer.push_back(Token(it_tok->second));
    ChunkBuffer.clear();
  }
  //todo If the word is not a keyword, it is a user defined variable? Maybe we
should
  //do this in the parser? For example what if we have "func"
*/
char Lexer::March() {
  if (start + ++HeadIndx <= end) {
    return start[HeadIndx];
  }
  bCompletedLex = true;
  return NULL;
}

char Lexer::GetCurrChar() { return start[HeadIndx]; }
void Lexer::GenMarch(bool (*func)(char), TOK_TYPES tok,
                     bool shouldSaveLiteral) {
  char curChar = GetCurrChar();
  while (func(curChar) && !bCompletedLex) {
    ChunkBuffer.push_back(curChar);
    curChar = March();
  }
  if (shouldSaveLiteral) {
    TokenBuffer.push_back(
        Token(tok, string(ChunkBuffer.begin(), ChunkBuffer.end())));
  } else {

    TokenBuffer.push_back(Token(tok));
  }
  ChunkBuffer.clear();
}
void Lexer::MarchWord() {
  auto func = [](char c) -> bool { return isalnum(c); };
  GenMarch(func, TOK_TYPES::WORD, true);
}

void Lexer::MarchNum() {
  auto func = [](char c) -> bool { return isdigit(c); };
  GenMarch(func, TOK_TYPES::DIGIT, true);
}

void Lexer::MarchSymbols() {
  // We want to continuously lex symbols. until there is a terminal symbol.
  // For example, a++ is valid. But " is terminal because there is no token that
  // extends that, if that makes sence.
  //!(isalnum(c) || isspace(c));

  set<string> ops = {"+", "++"};

  char curChar = GetCurrChar();
  while (!(isalnum(curChar) || isspace(curChar))) {
    int count = TrieTree.prefixCount(ChunkBuffer.begin(), ChunkBuffer.end());
  }
  ChunkBuffer.push_back(curChar);
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
    } else if (/*is special character*/) {
    }
  }
}

//  int32 i = 10++;
