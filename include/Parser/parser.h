#pragma once
#include <vector>

#include "Token/token.h"
#include "Token/tokenkinds.h"

namespace Brain {

struct ASTNode {
  Token& Value;
  ASTNode *LHS = nullptr;
  ASTNode *RHS = nullptr;
};

class AST {

private:
  ASTNode Root;
};
class Parser {

public:
  Parser() {}
  Parser(Token *begin, Token *end);

private:
  Token *Peek();
  Token *Consume(tok::TokenKind ExpectedToken);

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
  void Primary();
  void Type();

  std::vector<Token>* TokenBuffer = nullptr;
};
} // namespace Brain
