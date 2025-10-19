#pragma once

#include "tokenkinds.h"
#include <cstdint>
#include <string>
#include <string_view>

namespace Brain {

class Token {
public:
  Token() {}
  Token(tok::TokenKind type) : TokenType(type) {}
  Token(tok::TokenKind type, std::string_view literal)
      : TokenType(type), Literal(literal) {}
  static Token DetOpTok(std::string_view OpLiteral) {
    // Do binary search of operator tokens. Remember to do constexpr to sort the
    // tokens Also remember to have a special file for tokens
    return Token();
  }
  tok::TokenKind TokenType = tok::TokenKind::unknown;
  // std::vector<char> Literal;
  std::string Literal;
};
} // namespace Brain
