#include "Lexer/lexer.h"
#include <string>

int main() {
  std::string targetFile = "D:/Programming/B--/src/LexerTest.b";
  Brain::Lexer testLexer({targetFile});
  return 0;
}
