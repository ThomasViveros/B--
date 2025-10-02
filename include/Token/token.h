#pragma once

#include <cstdint>
// #include <vector>
#include <string>
#include <string_view>

namespace Brain {

enum class TOK_TYPES : uint32_t {
  UNK,
  Literal,
  Identifier,
  WORD,      // A word that may or may not be a keyword. TBD at parsing
  DIGIT,     // Number literal
  AST,       // *
  AMP,       // &
  MOD,       // %
  ADD,       // +
  SUB,       // -
  S_F_SLASH, // /
  S_B_SLASH, // \
  E_POINT,   // !
  Q_MARK,    // ?
  COLON,     // :
  SEMICOLON, // ;
  ASSIGN,    // =
  PERIOD,    // .
  S_QUOTE,   // '
  QUOTE,     // "

  L_PAREN,   // (
  R_PAREN,   // )
  L_BRACK,   // [
  R_BRACK,   // ]
  L_BRACE,   //{
  R_BRACE,   //}
  COM_START, // /*
  COM_END,   // */

  OR,        // ||
  AND,       // &&
  D_F_SLASH, // //
  D_COLON,   // ::
  EQUAL,     // ==
  ARROW,     // ->
  INC,       // ++
  DEC,       // --
  L_SHIFT,   // <<
  R_SHIFT,   // >>

  FUNC, // function keyword : func
  VOID, // type keyword : void
};

class Token {
public:
  Token() {}
  Token(TOK_TYPES type) : TokenType(type) {}
  Token(TOK_TYPES type, std::string_view literal)
      : TokenType(type), Literal(literal) {}
  static Token DetOpTok(std::string_view OpLiteral) {
    // Do binary search of operator tokens. Remember to do constexpr to sort the
    // tokens Also remember to have a special file for tokens
    return Token();
  }
  TOK_TYPES TokenType = TOK_TYPES::UNK;
  // std::vector<char> Literal;
  std::string Literal;
};
} // namespace Brain
