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

enum class EMarchState : uint8_t {
  Default,
  Word,
  Symbol,
  String,
  Comment,
  Number,
  Blank
};

class LexBuffer {

public:
  LexBuffer()=delete;
  explicit LexBuffer(size_t bufferSize);

  enum EAdvanceResult : uint8_t{
    cont, //We can continue
    eoc,  //We have hit the end of a chunk
    eof   //We have hit the end of file
  };

  bool HasCompleted()const;

  //Return is in regard to the next char
  EAdvanceResult GetCurrentChar(char& outChar) const;
  size_t GetPosition() const {return PeekOffset;}
  //Increment the pos and get the char at the new pos. Return is in regard to the next char (after advancing)
  EAdvanceResult Advance(char& outChar);
  //Increment the pos num times. outChars is all the chars starting from old pos up to and excluding the last pos.
  //Return is in regard to the new pos
  EAdvanceResult AdvanceMany(uint32_t num, std::vector<char>& outChars);
  //Get the char at pos + offset without incrementing pos. Return is in regard to the peeked position
  EAdvanceResult Peek(uint32_t offset, char& peekedChar) const;

  uint32_t GetRemainingElementSpace() const;

  char* GetWritingPosition() {
    //std::cout<< "WritingPosition Pos: "<<Pos<<std::endl;
    //std::cout<< "WritingPosition PeekOffset: "<<PeekOffset<<std::endl;

    return Buffer + Pos + PeekOffset;
  }
  size_t GetWritingAmount() {return BufferSize - PeekOffset;}
  void SetEOF(std::streamsize streamSize);

  //Expected to be called before refilling buffer
  void Compact();
  size_t PeekOffset = 0;
private:
  char* Buffer;
  size_t BufferSize = 0;

  size_t Pos = 0;
  //The position AFTER the last readable char
  std::streamsize EofPos = INDEX_NONE;
};

class Lexer {

  using int32 = int32_t;

public:
  Lexer();
  Lexer(const std::vector<std::string> &files, size_t lexBufferSize);

  int32 Lex(const std::string &targetFileName, const std::string &targetFilePath);

protected:
  bool issymbol(char c);

  void MarchWord();
  void MarchNum();
  void MarchSymbols();
  void MarchBlank();
  void MarchCommentLine();
  void MarchCommentBlock(){};
  void MarchString();

  // Create a token from ChunkBuffer and add it to the token buffer. This will clear the
  // ChunkBuffer(regardless if we save literal)
  void CreateToken(tok::TokenKind tokKind, bool bSaveLiteral);
  std::string_view ChunkBufferToStringView();

  // Where the lexer should start and end lexing (inclusive).
  const char *Start = nullptr;
  const char *End = nullptr;

  LexBuffer LexerBuffer;

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
  bool IsLexComplete() const {}
  bool bLastLexChunk = false;

  Trie TrieTree;

  // The identifier table this lexer owns
  IdentifierTable IdentifierTable;

  EMarchState MarchState = EMarchState::Default;

  //MarchNum func state
  bool bHasPeriodBeenFound = false;
  //End of MarchNum func state

  //MarchSymbols func state (Lives here to handle Lex chunk transitions)
  void ResetMarchSymbolsState();
  TrieNode *curNode = TrieTree.GetRoot();
  uint32_t numOfPeeks = 0;
  int32_t biggestPeekedIdentifierPos = INDEX_NONE;
  bool bPeekedEndOfChunk = false;
  //End of MarchSymbols func state

};

} // namespace Brain
#endif // BRAIN_LEXER_H
