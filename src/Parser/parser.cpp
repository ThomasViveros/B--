#include "Parser/parser.h"
#include <cstdint>
#include "Token/tokenkinds.h"
#include "Token/token.h"
using namespace Brain;

Token *Parser::Peek() {

}


void Parser::Type() {
    //First check for static or const

    //Then see if the token is a type
    switch (TokenBuffer->at(1).TokenType) {
        case tok::kw_int:
        case tok::kw_float:
        case tok::identifier:
        case tok::kw_bool:
        case tok::etc:
            break;
        default:
            //The token wasnt a type, return fail
            break;
    }
}
