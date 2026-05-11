#ifndef SWA_PARSER
#define SWA_PARSER
#include <ast/node.h>
#include <lexer/lexer.h>
#include <memory>
#include <string>
#include <vector>

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
  void parseDialect();
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
};
#endif // !SWA_PARSER
