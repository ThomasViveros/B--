#include "Lexer/lexer.h"
#include "Error/BrainFreeze/ErrorTypes.h"
#include "Lexer/IdentifierTable.h"
#include "Token/token.h"
#include "Token/tokenkinds.h"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace std;
using namespace Brain;
using namespace tok;

LexBuffer::EAdvanceResult LexBuffer::GetCurrentChar(char &outChar) const {
  outChar = Buffer[Pos];

  // EofPos may be the same as BufferSize, so do it first or else we could
  // incorrectly return eoc
  if (Pos + 1 == EofPos) {
    return eof;
  }
  if (Pos + 1 == BufferSize) {
    return eoc;
  }

  return cont;
}

LexBuffer::LexBuffer(size_t bufferSize) {
  BufferSize = bufferSize;
  Buffer = new char[bufferSize];
}

bool LexBuffer::HasCompleted() const {
  return Pos >= BufferSize || (EofPos != INDEX_NONE && Pos >= EofPos);
}

LexBuffer::EAdvanceResult LexBuffer::Advance(char &outChar) {
  outChar = Buffer[++Pos];

  // EofPos may be the same as BufferSize, so do it first or else we could
  // incorrectly return eoc
  if (Pos + 1 == EofPos) {
    return eof;
  }
  if (Pos + 1 == BufferSize) {
    return eoc;
  }

  return cont;
}

LexBuffer::EAdvanceResult LexBuffer::AdvanceMany(uint32_t num,
                                                 std::vector<char> &outChars) {
#ifndef NDEBUG
  if (num + Pos > BufferSize || (EofPos != INDEX_NONE && num + Pos > EofPos)) {
    throw std::out_of_range("Called AdvanceMany and went out of bounds!!!");
  }
#endif

  outChars.insert(outChars.end(), Buffer + Pos, Buffer + Pos + num);

  Pos += num;

  // EofPos may be the same as BufferSize, so do it first or else we could
  // incorrectly return eoc
  if (Pos == EofPos) {
    return eof;
  }
  if (Pos == BufferSize) {
    return eoc;
  }

  return cont;
}

LexBuffer::EAdvanceResult LexBuffer::Peek(uint32_t offset,
                                          char &peekedChar) const {
  if (EofPos != INDEX_NONE && offset + Pos >= EofPos) {
    return eof;
  }
  if (offset + Pos >= BufferSize) {
    return eoc;
  }

  peekedChar = Buffer[Pos + offset];

  return cont;
}

void LexBuffer::Compact() {
  memmove(Buffer, Buffer + Pos, BufferSize - Pos);

  Pos = 0;
  EofPos = INDEX_NONE;
}
void LexBuffer::SetEOFPosition(const streamsize streamSize) {
  EofPos = streamSize;
}

Lexer::Lexer(const std::vector<std::string> &files, size_t lexBufferSize)
    : LexerBuffer(lexBufferSize) {
  PopulateSymbolTree();

  for (auto &file : files) {
    std::ifstream ofile(file, std::ios::binary);

    if (!ofile.is_open()) {
      BrainFreeze::ErrorContext context(__FILE__, "", __LINE__, 0);
      BrainFreeze::RuntimeError newError("Unable to open file: " + file,
                                         context);
      ErrorBuffer.push_back(newError);
      continue;
    }

    // Determine the size of the file. Needed for the edge case where the number
    // of chars in the file is the same as the LexerBuffer size
    ofile.seekg(0, std::ios::end);
    std::streampos size = ofile.tellg();
    ofile.seekg(0, std::ios::beg);

    // Lex in chunks. This is so we can handle arbitrary sized files
    while (true) {
      if (ofile.read(LexerBuffer.GetWritingPosition(),
                     LexerBuffer.GetWritingAmount()) ||
          ofile.gcount() > 0) {
        // If we have exactly enough chars to fill the buffer, ofile.eof() will
        // be false, even if we have no more chars left to lex.
        if (ofile.eof() || ofile.tellg() == size) {
          LexerBuffer.SetEOFPosition(ofile.gcount() + LexerBuffer.PeekOffset);
        }
        // Buffer is populated, start lexing
        Lex(file, "TODO");
      } else {
        // We tried to read more but we had nothing more to lex
        break;
      }
    }

    for (auto &token : TokenBuffer) {
      cout << token.DEBUG_TOKEN();
    }
    for (auto &error : ErrorBuffer) {
      cout << error << endl;
    }
  }
}

bool Lexer::issymbol(char c) {
  // ASCII shenanigans
  return (33 <= c && c <= 47) || (58 <= c && c <= 64) || (91 <= c && c <= 96) ||
         (123 <= c && c <= 126);
}

void Lexer::CreateToken(tok::TokenKind tokKind, bool bSaveLiteral) {
  if (bSaveLiteral) {
    TokenBuffer.emplace_back(tokKind,
                             string(ChunkBuffer.begin(), ChunkBuffer.end()));
  } else {
    TokenBuffer.emplace_back(tokKind);
  }
  ChunkBuffer.clear();
}

std::string_view Lexer::ChunkBufferToStringView() {
  return {ChunkBuffer.data(), ChunkBuffer.size()};
}

void Lexer::PopulateSymbolTree() {
  // Populate the symbol trie tree.
#define PUNCTUATOR(NAME, VALUE) TrieTree.insert(VALUE);
#include "Token/tokenkinds.def"
#undef PUNCTUATOR
}

void Lexer::MarchWord() {

  char curChar;
  LexBuffer::EAdvanceResult advResult = LexerBuffer.GetCurrentChar(curChar);

  while (isalnum(curChar)) {
    ChunkBuffer.push_back(curChar);

    // We do this because we always want to advance but we dont want to override
    // the previous result.
    LexBuffer::EAdvanceResult tempResult = LexerBuffer.Advance(curChar);

    if (advResult == LexBuffer::EAdvanceResult::eoc) {
      // Don't make token or reset march state, we were cut off, wait for buffer
      // refill
      return;
    }
    if (advResult == LexBuffer::EAdvanceResult::eof) {
      // We hit the end of the file, break and make token
      break;
    }

    advResult = tempResult;
  }

  TokenKind keywordToken = unknown;
  if (IdentifierTable.IsIdentifier(ChunkBufferToStringView(),
                                   /*OUT*/ keywordToken)) {
    CreateToken(keywordToken, false);
  } else {
    // identifier
    CreateToken(tok::identifier, true);
  }

  MarchState = EMarchState::Default;
}

void Lexer::MarchNum() {
  // Number may be int or float, accept a single period and determine actual
  // literal type during semantic analysis.

  char curChar;

  LexBuffer::EAdvanceResult advResult = LexerBuffer.GetCurrentChar(curChar);

  while (isdigit(curChar) || (curChar == '.' && !bHasPeriodBeenFound)) {
    ChunkBuffer.push_back(curChar);

    if (curChar == '.') {
      bHasPeriodBeenFound = true;
    }

    // We do this because we always want to advance but we dont want to override
    // the previous result.
    LexBuffer::EAdvanceResult tempResult = LexerBuffer.Advance(curChar);

    if (advResult == LexBuffer::EAdvanceResult::eoc) {
      // Don't make token or reset march state, we were cut off, wait for buffer
      // refill
      return;
    }
    if (advResult == LexBuffer::EAdvanceResult::eof) {
      // We hit the end of the file, break and make token
      break;
    }

    advResult = tempResult;
  }

  CreateToken(TokenKind::numeric_constant, true);
  bHasPeriodBeenFound = false;
  MarchState = EMarchState::Default;
}

void Lexer::MarchSymbols() {
  /*Marching symbols is a bit more complicated that others because some symbols
   *can be a combo of punctuators. For e.g = is a token but so is == . So we
   *have to take that into account. We do so with peeking.
   *
   *
   *Also, the other march functions will try to completely resolve their string,
   *but in this func it is possible to leave even if there are symbols left.
   *This is to simplify the function. Our main loop will handle this and call
   *this function again if that is the case
   *
   **/
  char curChar;
  LexerBuffer.Peek(numOfPeeks, curChar);

  while (issymbol(curChar)) {
    curNode = curNode->GetChild(curChar);
    if (curNode) {
      if (curNode->children.empty()) {
        // There is no more possible moves we could do, attempt to make token
        if (curNode->bIsIdentifier) {
          // Make token, we are done
          biggestPeekedIdentifierPos = numOfPeeks;
          break;
        } else {
          throw std::logic_error("Symbol was the last element in trie chain "
                                 "yet it isnt marked identifier!!!");
        }
      } else {
        if (curNode->bIsIdentifier) {
          biggestPeekedIdentifierPos = numOfPeeks;
        }
      }
    } else {
      // We overshot
      if (biggestPeekedIdentifierPos == INDEX_NONE) {
        // We do this so we can advance, otherwise we would advance
        // INDEXNONE(aka -1) + 1 = 0
        biggestPeekedIdentifierPos = 0;
        // We never had an identifier, log error
        BrainFreeze::SyntaxError newError(
            "Invalid symbol: " +
                std::string(ChunkBuffer.begin(), ChunkBuffer.end()),
            BrainFreeze::ErrorContext(TargetFileName, TargetFilePath,
                                      CurrentLine + 1,
                                      CurrentColumn - ChunkBuffer.size()));

        ErrorBuffer.push_back(newError);
      }
      break;
    }

    LexBuffer::EAdvanceResult peekResult =
        LexerBuffer.Peek(++numOfPeeks, curChar);

    if (peekResult == LexBuffer::eoc) {
      // We tried to peek but we went out of bounds, wait for the buffer to
      // refill
      bPeekedEndOfChunk = true;
      LexerBuffer.PeekOffset = numOfPeeks;
      return;
    }
    if (peekResult == LexBuffer::eof) {
      break;
    }
  }

  LexerBuffer.AdvanceMany(biggestPeekedIdentifierPos + 1, ChunkBuffer);

  ResetMarchSymbolsState();

  // Attempt to make the token
  tok::TokenKind tokenKind;
  if (IdentifierTable.IsIdentifier(ChunkBufferToStringView(),
                                   /*OUT*/ tokenKind)) {
    // If this is a comment, dont waste time trying to tokenize, just
    // determine the end of the comment and discard.
    if (tokenKind == tok::TokenKind::slashslash) {
      // We detected a comment, discard these symbols and start marching the
      // comment line
      ChunkBuffer.clear();
      MarchState = EMarchState::Comment;
      return;
    } else if (tokenKind == TokenKind::quote) {
      // We detected a string, discard these symbols and start marching the
      // string
      ChunkBuffer.clear();
      MarchState = EMarchState::String;
      return;
    } else {
      CreateToken(tokenKind, false);
    }
  } else {
    // Throw error, we have a combination of symbols that dont make a keyword
    BrainFreeze::SyntaxError newError(
        "Symbols dont make up an identifier: " +
            std::string(ChunkBuffer.begin(), ChunkBuffer.end()),
        BrainFreeze::ErrorContext(TargetFileName, TargetFilePath,
                                  CurrentLine + 1,
                                  CurrentColumn - ChunkBuffer.size()));

    ErrorBuffer.push_back(newError);
    ChunkBuffer.clear();
  }

  MarchState = EMarchState::Default;
}
void Lexer::ResetMarchSymbolsState() {
  // Reset the state
  curNode = TrieTree.GetRoot();
  numOfPeeks = 0;
  biggestPeekedIdentifierPos = INDEX_NONE;
  LexerBuffer.PeekOffset = 0;
}

void Lexer::MarchBlank() {
  char curChar;

  LexBuffer::EAdvanceResult advResult = LexerBuffer.GetCurrentChar(curChar);

  while (isblank(curChar) || curChar == '\n') {

    if (curChar == '\n') {
      NewLine();
    }

    LexBuffer::EAdvanceResult tempResult = LexerBuffer.Advance(curChar);

    if (advResult == LexBuffer::EAdvanceResult::eoc) {
      // Don't reset march state, we were cut off, wait for buffer refill
      return;
    }
    if (advResult == LexBuffer::EAdvanceResult::eof) {
      // We hit the end of the file, break
      break;
    }

    advResult = tempResult;
  }

  MarchState = EMarchState::Default;
}
void Lexer::MarchCommentLine() {
  char curChar;

  LexBuffer::EAdvanceResult advResult = LexerBuffer.GetCurrentChar(curChar);

  while (curChar != '\n') {

    // We do this because we always want to advance but we dont want to override
    // the previous result.
    LexBuffer::EAdvanceResult tempResult = LexerBuffer.Advance(curChar);

    if (advResult == LexBuffer::EAdvanceResult::eoc) {
      // Don't reset march state, we were cut off, wait for buffer refill
      return;
    }
    if (advResult == LexBuffer::EAdvanceResult::eof) {
      // We hit the end of the file, break
      break;
    }

    advResult = tempResult;
  }

  if (advResult != LexBuffer::EAdvanceResult::eof) {
    // If we didn't hit the eof, that means we hit a newline.
    NewLine();
  }

  MarchState = EMarchState::Default;
}

void Lexer::MarchString() {
  char curChar;

  LexBuffer::EAdvanceResult advResult = LexerBuffer.GetCurrentChar(curChar);

  while (curChar != '"') {
    ChunkBuffer.push_back(curChar);

    // We do this because we always want to advance but we dont want to override
    // the previous result.
    LexBuffer::EAdvanceResult tempResult = LexerBuffer.Advance(curChar);

    if (advResult == LexBuffer::EAdvanceResult::eoc) {
      // Don't reset march state, we were cut off, wait for buffer refill
      return;
    }
    if (advResult == LexBuffer::EAdvanceResult::eof) {
      // We hit the end of the file, break
      break;
    }

    advResult = tempResult;
  }

  LexerBuffer.Advance(curChar);

  CreateToken(tok::string_literal, true);

  MarchState = EMarchState::Default;
}

int32 Lexer::Lex(const std::string &targetFileName,
                 const std::string &targetFilePath) {
  TargetFileName = targetFileName;
  TargetFilePath = targetFilePath;

  while (!LexerBuffer.HasCompleted() && !bPeekedEndOfChunk) {
    char curChar;
    LexerBuffer.GetCurrentChar(curChar);

    switch (MarchState) {
    case EMarchState::Word:
      MarchWord();
      break;
    case EMarchState::Number:
      MarchNum();
      break;
    case EMarchState::String:
      MarchString();
      break;
    case EMarchState::Comment:
      MarchCommentLine();
      break;
    case EMarchState::Symbol:
      MarchSymbols();
      break;
    case EMarchState::Blank:
      MarchBlank();
      break;
    case EMarchState::Default:
      if (isalpha(curChar)) {
        MarchState = EMarchState::Word;
        MarchWord();
      } else if (isdigit(curChar)) {
        MarchState = EMarchState::Number;
        MarchNum();
      } else if (isblank(curChar) || curChar == '\n') {
        MarchState = EMarchState::Blank;
        MarchBlank();
      } else if (issymbol(curChar)) {
        MarchState = EMarchState::Symbol;
        MarchSymbols();
      } else {
        LexerBuffer.Advance(curChar);
      }
      break;
    default:
      throw std::logic_error("Unhandled EMarchState case, perhaps you added or "
                             "renamed something in the enum?");
      break;
    }
  }

  bPeekedEndOfChunk = false;
  LexerBuffer.Compact();
  return 0;
}
