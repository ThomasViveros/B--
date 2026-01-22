#ifndef BRAIN_LEXER_H
#define BRAIN_LEXER_H

#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace Brain {

struct TrieNode {

  TrieNode *GetChild(char c) {
    children.contains(c);
    auto it = children.find(c);
    if (it != children.end()) {
      return it->second;
    }
    return nullptr;
  }

  std::unordered_map<char, TrieNode *> children;
  int count = 0; // how many combos pass through this node
  // Is this node an identifier (path included)
  bool bIsIdentifier = false;
};

class Trie {
public:
  Trie() : root(new TrieNode) {}
  TrieNode *GetRoot() { return root; }

  void insert(const std::string &word) {
    TrieNode *node = root;
    for (char c : word) {
      if (!node->children[c]) {
        node->children[c] = new TrieNode();
      }
      node = node->children[c];
      node->count++; // increment count for each prefix
    }
  }

  // Generic version: works with any iterable of chars
  template <typename Iter> int prefixCount(Iter begin, Iter end) const {
    TrieNode *node = root;
    for (Iter it = begin; it != end; ++it) {
      char c = *it;
      if (!node->children.count(c))
        return 0;
      node = node->children.at(c);
    }
    return node->count;
  }

private:
  TrieNode *root;
};

class Lexer {

  using int32 = int32_t;

public:
  Lexer() {}

protected:
  void Lex(char *start, char *end);

  bool issymbol(char c);

  char GetCurrChar();
  // Gets the next character in the file
  char March();
  void MarchBack(int32 amount = 1);
  void GenMarch(bool (*func)(char));
  void MarchWord();
  void MarchNum();
  void MarchSymbols();
  void MarchBlank();
  void MarchCommentLine();
  void MarchCommentBlock();
  void MarchString();

  // Create a token and add it to the token buffer. This will clear the
  // ChunkBuffer(regardless if we save literal)
  void CreateToken(tok::TokenKind tokKind, bool bSaveLiteral);
  std::string_view ChunkBufferToStringView();

  // Where the lexer should start and end lexing (inclusive).
  char *start = nullptr;
  char *end = nullptr;
  // Used while marching a chunk.
  std::vector<char> ChunkBuffer;

  std::vector<Token> TokenBuffer;
  int32 HeadIndx = 0;

private:
  bool bCompletedLex = false;
  static Trie TrieTree;
};

} // namespace Brain
#endif // BRAIN_LEXER_H
