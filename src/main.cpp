#include "Lexer/lexer.h"
#include "Token/token.h"
#include <iostream>
#include <string>

int main() {
  std::cout << "Hello" << std::endl;

  Brain::Lexer testLexer{};

  std::string testLexerString = "``";

  const char* a = testLexerString.c_str();
  const char* b = a + testLexerString.size();
  //std::cout << &a << std::endl;
  //std::cout << &b << std::endl;
  std::vector<Brain::Token> tokens = testLexer.Lex(a, b);
  for (auto& token : tokens ) {
    std::cout<<token.DEBUG_TOKEN();
  }
  return 0;
}
