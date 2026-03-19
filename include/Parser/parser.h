#pragma once
#include <cstdint>
#include <vector>

#include "Token/token.h"
#include "Token/tokenkinds.h"

namespace Brain {

enum ASTNodeKind : uint8_t {
  unknown,
  program,
  importDeclaration,
  classDeclaration,
  functionDeclaration,
  variableDeclaration,
  parameters,
  ifStatement,
  whileStatement,
  returnStatement,
  expression,
  assignment,
  logicOr,
  logicAnd,
  equality,
  comparison,
  term,
  factor,
  unary,
  primary,
  primarySuffix,
  memberAccess,
  arguments,
  type
};

// TODO
struct ASTNode {
  ASTNode() = default;
  ASTNode(ASTNodeKind nodeKind, Token *token = nullptr,
          const std::vector<ASTNode *> &children = {});
  ASTNodeKind Kind = unknown;
  // Some nodes may not have this set, instead relying on the nodes.
  Token *Value;
  // Some nodes may not have any children, instead just holding a value.
  std::vector<ASTNode *> ChildNodes;
};

class AST {

private:
  ASTNode Root;
};
// Whether or not the parse was successful
enum ParseResult : uint8_t { Succuess, Failure };
struct ParseOutcome {
  ParseOutcome(ParseResult result, int64_t index,
               ASTNode *generatedNode = nullptr)
      : Result(result), LastIndex(index), GeneratedNode(generatedNode) {}
  ParseResult Result = ParseResult::Failure;
  int64_t LastIndex = 0;
  ASTNode *GeneratedNode = nullptr;
};

/*
 * Parse rules. Each function will represent a language rule.
 * The way the functions will work is they will take the token index to start
 * on, and they will return a ParseOutcome wich will say whether or not the
 * parse failed and will have the next index to start parsing on.
 */

class Parser {

public:
  Parser() {}
  Parser(Token *begin, Token *end);

private:
  Token *Peek();
  Token *Consume(tok::TokenKind ExpectedToken);
  Token &GetTokenAtIndex(int64_t index) { return (*TokenBuffer)[index]; }
  ASTNode *GenerateASTNode(ASTNodeKind nodeKind, Token *token = nullptr,
                           const std::vector<ASTNode *> &children = {});
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

  ParseOutcome ArgList(int64_t index);
  ParseOutcome Primary(int64_t index);
  ParseOutcome PrimarySuffix(int64_t index);
  ParseOutcome Type(int64_t index);

  std::vector<Token> *TokenBuffer = nullptr;
};
} // namespace Brain
