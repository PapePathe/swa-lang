#ifndef SWA_PARSER
#define SWA_PARSER

#include <ast/node.h>
#include <lexer/lexer.h>

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class ParserException : public std::exception {
  std::string message;
  Span span;
  std::string labelText;
  std::string suggestionText;

public:
  ParserException(std::string msg, Span s, std::string label = "",
                  std::string suggestion = "")
      : message(std::move(msg)), span(s), labelText(std::move(label)),
        suggestionText(std::move(suggestion)) {}

  const char *what() const noexcept override { return "Swa Syntax Error"; }
  std::string getMessage() const { return message; }
  std::string getLabelText() const { return labelText; }
  Span getSpan() const { return span; }

  // This method handles the entire Rust-style visual layout internally
  void emitDiagnostic(const SourceManager &sm) const {
    auto [line, col] = sm.getLineCol(span.start);
    auto [_, endCol] = sm.getLineCol(span.end);

    // 1. Core Header Block (Bold Red formatting)
    std::cout << "\033[1;31merror\033[0m: " << message << "\n";
    std::cout << "  --> swa_source:" << line << ":" << col << "\n";

    // 2. Code Frame Context Line
    std::cout << "   |\n";
    std::cout << " " << line << " | " << sm.getLineSnippet(line) << "\n";

    // 3. Highlight Carets (^^^^) underneath the exact bytes
    std::cout << "   | ";
    for (size_t i = 1; i < col; ++i)
      std::cout << " ";
    for (size_t i = col; i <= endCol; ++i)
      std::cout << "\033[1;31m^\033[0m";

    if (!labelText.empty()) {
      std::cout << " " << labelText;
    }
    std::cout << "\n";

    // 4. Cyan Help / Suggestion Block
    if (!suggestionText.empty()) {
      std::cout << "   | \033[1;36mhelp\033[0m: " << suggestionText << "\n";
    }
    std::cout << "\n";
  }
};
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
#endif // !SWA_PARSER
