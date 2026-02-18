#ifndef BRAIN_LEXER_H
#define BRAIN_LEXER_H

#include "Error/BrainFreeze/ErrorTypes.h"
#include "IdentifierTable.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
  Trie() = default;
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

struct LexSummary {
  LexSummary() = default;
  LexSummary(const std::vector<Token> &tokens,
             const std::vector<BrainFreeze::Error> &errors)
      : Tokens(tokens), Errors(errors) {}
  std::vector<Token> Tokens;
  std::vector<BrainFreeze::Error> Errors;
};

enum class MarchResult : uint8_t {
  cont, // Continue to march
  stop, // Stop marching due to reaching the end of MarchChunk
  eoc,  // Stop marching due to reaching end of LexChunk
  eof   // Stop marching due to reaching end of file
};

enum class MarchState : uint8_t {
  Default,
  Word,
  Symbol,
  String,
  Comment,
  Number
};

class LexBuffer {
public:
  LexBuffer(size_t bufferSize);

  char Advance();
  char peek(uint32_t offset);

  void Append(std::string_view newChars);

private:
  void Compact();
  size_t Pos = 0;
  char *Buffer;
};

class Lexer {

  using int32 = int32_t;

public:
  Lexer();
  Lexer(const std::vector<std::string> &files);

  int32 Lex(const char *start, const char *end,
            const std::string &targetFileName,
            const std::string &targetFilePath, bool bIsLastLexChunk = false);

protected:
  bool issymbol(char c);

  char GetCurrChar() const;
  // Gets the next character in the file
  MarchResult March(char &c, bool saveToChunkBuffer = true);
  MarchResult GenMarch(bool (*func)(char), bool saveToChunkBuffer = true);
  void MarchWord();
  void MarchNum();

  // If we hit the end of a lex chunk(that isn't EOF) while lexing symbols, we
  // have to backtrack the lex chunk so that the next lex chunk starts at the
  // beginning of the symbols. This is because we may need to backtrack during
  // the symbol lexing, but we won't be able to through lex chunks.
  void MarchSymbols(int32 &lexChunkBackTrackAmount);
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
  // The current char we are on
  char *Head = nullptr;
  // WARNING: This will cause slicing, as of right now it is compatible with our
  // error types but if we ever want specialized information on any error type
  // we will have to refactor this
  std::vector<BrainFreeze::Error> ErrorBuffer;

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
  void PopulateSymbolTree();
  // Returns true if The head is greater or equal to the end
  bool IsLexComplete() const { return Head >= End; }
  bool bLastLexChunk = false;

  Trie TrieTree;

  // The identifier table this lexer owns
  IdentifierTable IdentifierTable;
};

} // namespace Brain
#endif // BRAIN_LEXER_H
