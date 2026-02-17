#include "Lexer/IdentifierTable.h"

#include <iostream>

#include "Core/Exceptions.h"
#include <string_view>
#include <bits/ostream.tcc>

using namespace Brain;

bool IdentifierTable::IsIdentifier(std::string_view word, tok::TokenKind &tokenKind) {
    if (const IdentifierInfo* II = TryGet(word)) {
        tokenKind = II->TokenId;
        return true;
    }

  return false;
}

static void AddKeyword(std::string_view keyword, tok::TokenKind tokenCode, IdentifierTable &table) {

    IdentifierInfo& info = table.Get(keyword, tokenCode);
    //TODO: do stuff to identifier info
    info.bIsKeyword = true;
}

void IdentifierTable::AddKeywords() {

#define KEYWORD(NAME, FLAGS) AddKeyword(std::string_view(#NAME), tok::kw_ ## NAME, *this);
#include "Token/tokenkinds.def"
#undef KEYWORD

#define PUNCTUATOR(NAME, VALUE) AddKeyword(std::string_view(VALUE), tok::NAME, *this);
#include "Token/tokenkinds.def"
#undef PUNCTUATOR
}