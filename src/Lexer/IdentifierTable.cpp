#include "Lexer/IdentifierTable.h"
#include "Core/Exceptions.h"
#include <string_view>


using namespace Brain;


bool IdentifierTable::IsKeyword(std::string_view word, tok::TokenKind& tokenKind) {
    throw NotImplemented();

    return false;
}


static void AddKeyword(std::string_view Keyword, IdentifierTable& Table) {

}

void IdentifierTable::AddKeywords() {

#define KEYWORD(NAME, FLAGS) \
    AddKeyword(std::string_view(#NAME), *this);
}
