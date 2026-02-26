#include "Lexer/lexer.h"
#include <string>

int main() {
  std::string targetFile = "D:/Programming/B--/src/LexerTest.b";
  Brain::Lexer testLexer({targetFile},7);
  std::cout<<"Full file test====="<<std::endl;
  Brain::Lexer testLexer2({targetFile},100);
  return 0;
}
