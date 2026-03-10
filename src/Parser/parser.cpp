#include "Parser/parser.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cstdint>
using namespace Brain;

Token *Parser::Peek() {}

Parser::ParseResult Parser::Type(int64_t index) {
  // First check for const
  if (TokenBuffer->at(index).TokenType == tok::kw_const) {
    index++;
  }
  // Then see if the token is a type
  switch (TokenBuffer->at(index).TokenType) {
  case tok::kw_int:
  case tok::kw_float:
  case tok::identifier:
  case tok::kw_bool:
  case tok::kw_char:
    return ParseResult::Succuess;
  default:
    // The token wasnt a type, return fail
    return ParseResult::Failure;
  }
  return ParseResult::Failure;
}
