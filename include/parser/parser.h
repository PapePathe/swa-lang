#pragma once

#include <ast/node.h>
#include <lexer/lexer.h>
#include <parser/exception.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Parser {
  std::vector<Token> tokens;
  size_t pos = 0;

  Token current();
  Token previous();
  Token expect(TokenType type);
  Token expect(TokenType type,
               std::function<ParserException(Span errorSpan)> errorHandler);
  bool isAtEnd();

public:
  Parser(std::vector<Token> t) : tokens(t) {}

  std::vector<std::unique_ptr<Test_Expr>> Tests();
  std::unique_ptr<BlockExpr> parseProgram();

private:
  std::vector<std::unique_ptr<Test_Expr>> tests;

  void trace(std::string msg);
  void debug(std::string msg);
  void parseDialect();
  std::unique_ptr<Expr> parseArrayAccess(std::unique_ptr<Expr> sequence);
  std::unique_ptr<Expr> parseArrayInitExpr();
  std::unique_ptr<Expr> parseStatement();
  std::unique_ptr<Expr> parsePrimary();
  std::unique_ptr<Expr> parseIf();
  std::unique_ptr<Expr> parsePrintFormatted();
  std::unique_ptr<Expr> parsePrint();
  std::unique_ptr<Expr> parseReturn();
  std::unique_ptr<Expr> parseStruct();
  std::unique_ptr<Type> parseType();
  std::unique_ptr<Expr> parseFunction();
  std::unique_ptr<BlockExpr> parseBlock();
  std::unique_ptr<Expr> parseDeclaration();
  std::unique_ptr<Expr> createBinaryNode(TokenType op,
                                         std::unique_ptr<Expr> left,
                                         std::unique_ptr<Expr> right);
  std::unique_ptr<Expr> parseEquals(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseLessThan(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseLessThanEquals(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseGreaterThanEquals(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseGreaterThan(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseFunctionCall(std::unique_ptr<Expr> callee);
  bool isCompareOp(TokenType t);
  int getPrecedence(TokenType type);
  std::unique_ptr<Expr> parseExpression(int = 0);
  void parseTest();
  std::unique_ptr<Expr> parseAsserts();
};
