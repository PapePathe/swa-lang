#include "lexer/tokentype.hpp"
#include <ast/node.h>
#include <lexer/lexer.hpp>
#include <memory>
#include <string>
#include <vector>

#ifndef SWA_PARSER
#define SWA_PARSER
class Parser {
  std::vector<Token> tokens;
  size_t pos = 0;

  Token current();
  Token expect(TokenType type);
  bool isAtEnd();

public:
  Parser(std::vector<Token> t) : tokens(t) {}

  std::unique_ptr<BlockExpr> parseProgram();

private:
  void trace(std::string msg);
  void debug(std::string msg);
  std::unique_ptr<Expr> parseStatement();
  void parseDialect();
  std::unique_ptr<Expr> parseIf();
  std::unique_ptr<Expr> parsePrint();
  std::unique_ptr<Expr> parseStruct();
  std::unique_ptr<Type> parseType();
  std::unique_ptr<Expr> parseFunction();
  std::unique_ptr<BlockExpr> parseBlock();
  std::unique_ptr<Expr> parseDeclaration();
  std::unique_ptr<Expr> parseEquals(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseLessThan(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseLessThanEquals(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseGreaterThanEquals(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parseGreaterThan(std::unique_ptr<Expr> left);
  bool isCompareOp(TokenType t);
  std::unique_ptr<Expr> parseExpression(std::unique_ptr<Expr> left = nullptr);
};
#endif // !SWA_PARSER
