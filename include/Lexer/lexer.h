#ifndef BRAIN_LEXER_H
#define BRAIN_LEXER_H

#include "Token/token.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace Brain {

struct TrieNode {
  std::unordered_map<char, TrieNode *> children;
  int count = 0; // how many combos pass through this node
};

class Trie {
public:
  Trie() : root(new TrieNode) {}

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

  // Alias for Vector of characters
  using int32 = int32_t;

public:
  Lexer() {}

protected:
  void Lex(char *start, char *end);

  bool issymbol(char c);

  char GetCurrChar();
  // Gets the next character in the file
  char March();
  void GenMarch(bool (*func)(char), TOK_TYPES tok, bool shouldSaveLiteral);
  void MarchWord();
  void MarchNum();
  void MarchSymbols();
  void MarchBlank();
  void MarchCommentLine();
  void MarchCommentBlock();

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
