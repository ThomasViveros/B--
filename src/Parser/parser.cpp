#include "Parser/parser.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cstdint>
using namespace Brain;

Token *Parser::Peek() {}

ParseOutcome Parser::Primary(int64_t index) {

  // primary	        :=	NUMBER | STRING | "true" | "false" | IDENT
  // primarySuffix | "(" expr ")"
  switch (TokenBuffer->at(index).TokenType) {
  case tok::numeric_constant:
  case tok::string_literal:
  case tok::kw_true:
  case tok::kw_false:
  case tok::identifier:
    // see if there is a suffix
    PrimarySuffix();
    break;
  case tok::identifier:
    break;
  case tok::l_paren:
    break;
  }
  return ParseResult::Failure;
}

ParseOutcome Parser::PrimarySuffix(int64_t index) {

  // primarySuffix   := "(" [ argList ] ")" { memberAccess} | { memberAccess }
  if (GetTokenAtIndex(index).TokenType != tok::l_paren) {
    return ParseResult::Failure;
  }
  // Optional argument list
  // TODO: we have to track how far the index went
  index = ArgList(++index).Index;

  if (GetTokenAtIndex(index).TokenType != tok::r_paren) {
    return ParseOutcome(Failure, index);
  }

  return ParseOutcome(Failure, index);
}
ParseOutcome Parser::ArgList(int64_t index) {
  // arg_list := primary { "," primary }
  Primary(index);
}
ParseOutcome Parser::Type(int64_t index) {
  // First check for const
  if (TokenBuffer->at(index).TokenType == tok::kw_const) {
    index++;
  }
  // Then see if the token is a type
  switch (TokenBuffer->at(index).TokenType) {
  case tok::kw_int:
  case tok::kw_float:
  case tok::kw_bool:
  case tok::kw_char:
  case tok::identifier:
    return ParseOutcome(Succuess, index);
  default:
    // The token wasnt a type, return fail
    return ParseOutcome(Failure, index);
  }
}
