#pragma once
#include <string_view>
#include "Token/tokenkinds.h"

namespace Brain {


class IdentifierTable {

public:
    static IdentifierTable& GetIdentifierTable() {
        static IdentifierTable instance;
        return instance;
    }

    bool IsKeyword(std::string_view word, tok::TokenKind& tokenKind);

protected:
    void AddKeywords();

private:
};


}// end namespace Brain