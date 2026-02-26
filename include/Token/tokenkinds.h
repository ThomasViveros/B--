//===--- TokenKinds.h - Enum values for C Token Kinds -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the clang::TokenKind enum and support functions.
///
//===----------------------------------------------------------------------===//
#pragma once
#include <string_view>


namespace Brain {

namespace tok {

#ifndef NDEBUG
  constexpr std::string_view debugTokArr[] = {
#define TOK(X) std::string_view{#X},
#include "Token/tokenkinds.def"
#undef TOK
  };
#endif

/// Provides a simple uniform namespace for tokens from all C languages.
enum TokenKind : unsigned short {
#define TOK(X) X,
#include "Token/tokenkinds.def"
#undef TOK
  NUM_TOKENS
};

/// Determines the name of a token as used within the front end.
///
/// The name of a token will be an internal name (such as "l_square")
/// and should not be used as part of diagnostic messages.
const char *getTokenName(TokenKind Kind);

/// Determines the spelling of simple punctuation tokens like
/// '!' or '%', and returns NULL for literal and annotation tokens.
///
/// This routine only retrieves the "simple" spelling of the token,
/// and will not produce any alternative spellings (e.g., a
/// digraph). For the actual spelling of a given Token, use
/// Preprocessor::getSpelling().
const char *getPunctuatorSpelling(TokenKind Kind);

/// Determines the spelling of simple keyword and contextual keyword
/// tokens like 'int' and 'dynamic_cast'. Returns NULL for other token kinds.
const char *getKeywordSpelling(TokenKind Kind);

/// Return true if this is a raw identifier or an identifier kind.
inline bool isAnyIdentifier(TokenKind K) {
  return (K == tok::identifier) || (K == tok::raw_identifier);
}

/// Return true if this is a C or C++ string-literal (or
/// C++11 user-defined-string-literal) token.
inline bool isStringLiteral(TokenKind K) {
  return K == tok::string_literal || K == tok::wide_string_literal ||
         K == tok::utf8_string_literal || K == tok::utf16_string_literal ||
         K == tok::utf32_string_literal;
}

/// Return true if this is a "literal" kind, like a numeric
/// constant, string, etc.
inline bool isLiteral(TokenKind K) {
  const bool isInLiteralRange =
      K >= tok::numeric_constant && K <= tok::utf32_string_literal;

  return isInLiteralRange;
}

/// Return true if this is any of tok::annot_* kinds.
bool isAnnotation(TokenKind K);

/// Return true if this is an annotation token representing a pragma.
bool isPragmaAnnotation(TokenKind K);

} // end namespace tok
} // end namespace Brain
