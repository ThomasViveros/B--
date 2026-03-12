#pragma once
#include <cstdint>
#include <vector>

#include "Token/token.h"
#include "Token/tokenkinds.h"

namespace Brain {

struct ASTNode {
  Token &Value;
  ASTNode *LHS = nullptr;
  ASTNode *RHS = nullptr;
};

class AST {

private:
  ASTNode Root;
};
// Whether or not the parse was successful
enum ParseResult : uint8_t { Succuess, Failure };
struct ParseOutcome {
  ParseOutcome(ParseResult result, int64_t index)
      : Result(result), LastIndex(index) {}
  ParseResult Result = ParseResult::Failure;
  int64_t LastIndex = 0;
};

class Parser {

public:
  Parser() {}
  Parser(Token *begin, Token *end);

private:
  Token *Peek();
  Token *Consume(tok::TokenKind ExpectedToken);
  Token &GetTokenAtIndex(int64_t index) { return (*TokenBuffer)[index]; }
  /*
   * These are the grammar rules described in the EBNF
   * */
  void Program();
  void Decl();
  void FunDecl();
  void Params();
  void Param();
  void VarDecl();
  void Block();
  void Stmt();
  void ExprStmt();
  void IfStmt();
  void WhileStmt();
  void ReturnStmt();
  void Expr();
  void Assignment();
  void LogicOr();
  void LogicAnd();
  void Equality();
  void Comparison();
  void Term();
  void Factor();
  void Unary();
  ParseOutcome Primary(int64_t index);
  ParseOutcome PrimarySuffix(int64_t index);
  ParseOutcome Type(int64_t index);

  std::vector<Token> *TokenBuffer = nullptr;
};
} // namespace Brain
