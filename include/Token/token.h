#pragma once

#include "tokenkinds.h"
#include <format>
#include <string>
#include <string_view>

namespace Brain {

class Token {
public:
  Token() = default;
  explicit Token(tok::TokenKind type) : TokenType(type) {}
  Token(tok::TokenKind type, std::string_view literal)
      : TokenType(type), Literal(literal) {}

  tok::TokenKind TokenType = tok::TokenKind::unknown;
  // std::vector<char> Literal;
  std::string Literal;

    [[nodiscard]] std::string DEBUG_TOKEN() const {
        return std::format("Kind:{}, Literal: {} \n",static_cast<unsigned short>(TokenType), Literal);
    }
};
} // namespace Brain
