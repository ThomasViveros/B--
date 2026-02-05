#pragma once
#include "Token/tokenkinds.h"
#include <string_view>
#include <string>
#include <unordered_map>

namespace Brain {

  struct IdentifierInfo {
    //TODO

    bool bIsKeyword = false;
    tok::TokenKind TokenId;
  };

  class IdentifierTable {


    public:


    IdentifierTable() {
      AddKeywords();
    }


    bool IsIdentifier(std::string_view word, tok::TokenKind& tokenKind);

    IdentifierInfo& Get(std::string_view Name) {
      auto& emplaceResult = *Identifiers.try_emplace(std::string(Name), IdentifierInfo()).first;
      return emplaceResult.second;
    }

    IdentifierInfo& Get(std::string_view Name, tok::TokenKind& tokenCode) {
      IdentifierInfo& II = Get(Name);
      II.TokenId = tokenCode;
      return II;
    }

    //TODO: Rename, maybe to find or something idk
    IdentifierInfo* TryGet(std::string_view Name) {
      auto it = Identifiers.find(std::string(Name));
      if (it != Identifiers.end()) {
        return &it->second;
      }
      return nullptr;
    }
  protected:
    //void AddIdentifier();

  private:
    void AddKeywords();

    std::unordered_map<std::string, IdentifierInfo> Identifiers;
  };

} // end namespace Brain
