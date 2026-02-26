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

struct PunctuatorTrieNode {

  PunctuatorTrieNode *GetChild(const char c) {
    if (const auto it = children.find(c); it != children.end()) {
      return it->second;
    }
    return nullptr;
  }

  std::unordered_map<char, PunctuatorTrieNode *> children;
  // Is this node an identifier (path included)
  bool bIsIdentifier = false;
};

class PunctuatorTrie {
public:
  Trie() = default;
  PunctuatorTrieNode *GetRoot() { return &root; }

  void insert(const std::string &word) {
    PunctuatorTrieNode *node = &root;
    for (char c : word) {
      if (!node->children[c]) {
        node->children[c] = new PunctuatorTrieNode();
      }
      node = node->children[c];
    }
    node->bIsIdentifier = true;
  }

private:
  PunctuatorTrieNode root;
};

struct LexSummary {
  LexSummary() = default;
  LexSummary(const std::vector<Token> &tokens,
             const std::vector<BrainFreeze::Error> &errors)
      : Tokens(tokens), Errors(errors) {}
  std::vector<Token> Tokens;
  std::vector<BrainFreeze::Error> Errors;
};
// Represents what we are currenlty lexing.
enum class EMarchState : uint8_t {
  Default, // Undetermined
  Word,    // Alphabetic words. Could be Identifiers, Keywords, etc
  Symbol,  // E.G: * + - @
  String,
  Comment,
  Number,
  Blank
};

class LexBuffer {

public:
  LexBuffer() = delete;
  explicit LexBuffer(size_t bufferSize);

  enum EAdvanceResult : uint8_t {
    cont, // We can continue
    eoc,  // We have hit the end of a chunk
    eof   // We have hit the end of file
  };
  // Returns wether or not getting the current char would result in a failure.
  // I.E. if we reach eoc or eof
  bool HasCompleted() const;

  // Return is in regard to the next char
  EAdvanceResult GetCurrentChar(char &outChar) const;
  // Increment the pos and get the char at the new pos. Return is in regard to
  // the next char (after advancing)
  EAdvanceResult Advance(char &outChar);
  // Increment the pos num times. outChars is all the chars starting from old
  // pos up to and excluding the last pos. Return is in regard to the new pos
  EAdvanceResult AdvanceMany(uint32_t num, std::vector<char> &outChars);
  // Get the char at pos + offset without incrementing pos. Return is in regard
  // to the peeked position
  EAdvanceResult Peek(uint32_t offset, char &peekedChar) const;
  // Returns the position to start filling up the buffer
  char *GetWritingPosition() { return Buffer + Pos + PeekOffset; }
  // Returns the number of characters we need to fill up the buffer.
  size_t GetWritingAmount() { return BufferSize - PeekOffset; }
  // Marks the eof position for this buffer. Only do this when on the last lex
  // chunk. (EOFPos should be the position AFTER the last char)
  void SetEOFPosition(std::streamsize streamSize);

  // Shifts the remaining chars in the buffer to the front and resets
  // state.Expected to be called before refilling buffer
  void Compact();
  // This variable is a bit specific but its meant to be used for the symbols
  // (because as of now its the only thing that peeks). When peeking, if we hit
  // the eoc we want to stop what we are doing and wait for the buffer to
  // refill. This variable will save that state so we can pick up where we left
  // off. Also, when it comes to refilling the buffer, we dont want to overwrite
  // the chars that weve compacted. So we use this variable to determine where
  // we should start writing. This variable should probably be in the Lexer
  // under the symbols state section. But perhaps its fine to leave it here if
  // we ever use peeking in any other march.
  size_t PeekOffset = 0;

private:
  // The Buffer contents.
  char *Buffer;
  // The amount of characters this buffer can fit
  size_t BufferSize = 0;
  // The position of the current char
  size_t Pos = 0;
  // The position AFTER the last readable char
  std::streamsize EofPos = INDEX_NONE;
};

class Lexer {

public:
  Lexer() = delete;
  Lexer(const std::vector<std::string> &files, size_t lexBufferSize);

  int32 Lex(const std::string &targetFileName,
            const std::string &targetFilePath);

protected:
  bool issymbol(char c);

  // These march functions will lex their corresponding character blocks and try
  // to make tokens
  void MarchWord();
  void MarchNum();
  void MarchSymbols();
  void MarchBlank();
  void MarchCommentLine();
  void MarchCommentBlock(){};
  void MarchString();

  // Create a token from ChunkBuffer and add it to the token buffer. This will
  // clear the ChunkBuffer(regardless if we save literal)
  void CreateToken(tok::TokenKind tokKind, bool bSaveLiteral);
  std::string_view ChunkBufferToStringView();

  LexBuffer LexerBuffer;

  // Temporary buffer used while marching a chunk.
  std::vector<char> ChunkBuffer;
  // The accumalated tokens generated during lexing.
  std::vector<Token> TokenBuffer;

  // WARNING: This will cause slicing, as of right now it is compatible with our
  // error types but if we ever want specialized information on any error type
  // we will have to refactor this
  std::vector<BrainFreeze::Error> ErrorBuffer;

  // Used to track what line number we are currently on. Helpfull for debugging
  void NewLine() {
    CurrentLine++;
    CurrentColumn = 0;
  };
  // The current line we are on
  int32 CurrentLine = 0;
  // AKA the index of the char on the line
  int32 CurrentColumn = 0;

  // The name of the file we are currently lexing
  std::string TargetFileName;
  // The full path of the file we are currently lexing
  std::string TargetFilePath;

private:
  void PopulateSymbolTree();

  PunctuatorTrie TrieTree;

  // The identifier table this lexer owns
  IdentifierTable IdentifierTable;
  // The current march state. Used for keeping track what march state we were
  // last on during lex chunk transitions
  EMarchState MarchState = EMarchState::Default;

  // MarchNum func state (Lives here to handle Lex chunk transitions)
  bool bHasPeriodBeenFound = false;
  // End of MarchNum func state

  // MarchSymbols func state (Lives here to handle Lex chunk transitions)
  // Resets the marchsymbols state to default
  void ResetMarchSymbolsState();
  PunctuatorTrieNode *curNode = TrieTree.GetRoot();
  uint32_t numOfPeeks = 0;
  int32_t biggestPeekedIdentifierPos = INDEX_NONE;
  bool bPeekedEndOfChunk = false;
  // End of MarchSymbols func state
};

} // namespace Brain
#endif // BRAIN_LEXER_H
