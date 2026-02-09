#ifndef BRAIN_LEXER_H
#define BRAIN_LEXER_H

#include "IdentifierTable.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Core/CoreTypes.h"

namespace Brain {

struct TrieNode {

  TrieNode *GetChild(const char c) {
    if (const auto it = children.find(c); it != children.end()) {
      return it->second;
    }
    return nullptr;
  }

  TrieNode *GetParent(int32_t generations = 1) {
    TrieNode *parent = this;
    for (int i = 0; i < generations; ++i) {
      parent = parent->parent;
    }
    return parent;
  }

  TrieNode *parent = nullptr;
  std::unordered_map<char, TrieNode *> children;
  int count = 0; // how many combos pass through this node
  // Is this node an identifier (path included)
  bool bIsIdentifier = false;
};

class Trie {
public:
  Trie() {}
  TrieNode *GetRoot() { return &root; }

  void insert(const std::string &word) {
    TrieNode *node = &root;
    for (char c : word) {
      if (!node->children[c]) {
        node->children[c] = new TrieNode();
      }
      node = node->children[c];
      node->parent = node;
      node->count++; // increment count for each prefix
    }
    node->bIsIdentifier = true;
  }

  // Generic version: works with any iterable of chars
  template <typename Iter> int prefixCount(Iter begin, Iter end) const {
    TrieNode node = root;
    for (Iter it = begin; it != end; ++it) {
      char c = *it;
      if (!node.children.contains(c))
        return 0;
      node = *node.children.at(c);
    }
    return node.count;
  }

private:
  TrieNode root;
};

class Lexer {

  using int32 = int32_t;

public:
  Lexer();

  std::vector<Token> Lex(const char *start, const char *end,
                         const std::string &targetFileName,
                         const std::string &targetFilePath);

protected:
  bool issymbol(char c);

  char GetCurrChar() const;
  // Gets the next character in the file
  bool March(char &c, bool saveToChunkBuffer = true);
  void GenMarch(bool (*func)(char), bool saveToChunkBuffer = true);
  void MarchWord();
  void MarchNum();
  void MarchSymbols();
  void MarchBlank();
  void MarchCommentLine();
  void MarchCommentBlock(){};
  void MarchString();

  // Create a token and add it to the token buffer. This will clear the
  // ChunkBuffer(regardless if we save literal)
  void CreateToken(tok::TokenKind tokKind, bool bSaveLiteral);
  std::string_view ChunkBufferToStringView();

  // Where the lexer should start and end lexing (inclusive).
  const char *Start = nullptr;
  const char *End = nullptr;
  // Used while marching a chunk.
  std::vector<char> ChunkBuffer;

  std::vector<Token> TokenBuffer;
  int32 HeadIndx = 0;

  void NewLine() {
    CurrentLine++;
    CurrentColumn = 0;
  };
  int32 CurrentLine = 0;
  // AKA the index of the char on the line
  int32 CurrentColumn = 0;

  std::string TargetFileName;
  std::string TargetFilePath;

private:
  bool bCompletedLex = false;

  Trie TrieTree;

  // The identifier table this lexer owns
  IdentifierTable IdentifierTable;
};

} // namespace Brain
#endif // BRAIN_LEXER_H
