#pragma once

#include <cstdint>
//#include <vector>
#include <string>

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
  Token(TOK_TYPES type, std::string literal)
      : TokenType(type), Literal(literal) {}
  TOK_TYPES TokenType = TOK_TYPES::UNK;
  // std::vector<char> Literal;
  std::string Literal;
};
