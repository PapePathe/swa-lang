#include "lexer/tokentype.hpp"
#include <algorithm>
#include <ast/node.hpp>
#include <lexer/lexer.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Parser {
  std::vector<Token> tokens;
  size_t pos = 0;

  // Helper: Get current token
  Token current() { return tokens[pos]; }

  // Helper: Consume token if it matches type, else throw error
  Token expect(TokenType type) {
    if (current().type == type)
      return tokens[pos++];
    throw std::runtime_error("Unexpected token: ```" + current().value + "```");
  }

  bool isAtEnd() {
    return pos >= tokens.size() || current().type == TokenType::END_OF_FILE;
  }

public:
  Parser(std::vector<Token> t) : tokens(t) {}

  std::unique_ptr<BlockExpr> parseProgram() {
    std::vector<std::unique_ptr<Expr>> stmts;
    while (!isAtEnd()) {
      stmts.push_back(parseStatement());
    }
    return std::make_unique<BlockExpr>(std::move(stmts));
  }

private:
  void trace(std::string msg) { std::cout << "PARSER TRACE " << msg; }
  void debug(std::string msg) {}
  std::unique_ptr<Expr> parseStatement() {
    trace("begin parse statement\n");

    if (current().type == TokenType::FUNCTION)
      return parseFunction();
    if (current().type == TokenType::STRUCT)
      return parseStruct();
    if (current().type == TokenType::PRINT)
      return parsePrint();
    if (current().type == TokenType::LET)
      return parseDeclaration();
    if (current().type == TokenType::IF)
      return parseIf();

    trace("end parse statement\n");

    return parseExpression();
  }

  std::unique_ptr<Expr> parseIf() {
    trace("begin parse if\n");
    expect(TokenType::IF);

    trace("begin parse if condition\n");
    auto cond = parseExpression();
    trace("end parse if condition\n");

    auto b = parseBlock();

    trace("end parse if\n");

    std::unique_ptr<BlockExpr> e;
    if (current().type == TokenType::ELSE) {
      trace("begin parse else condition\n");
      expect(TokenType::ELSE);
      e = parseBlock();
      trace("end parse else condition\n");
    }

    return std::make_unique<IfExpr>(std::move(cond), std::move(b),
                                    std::move(e));
  }

  std::unique_ptr<Expr> parsePrint() {
    expect(TokenType::PRINT); // 'print'
    expect(TokenType::OPEN_PAREN);
    std::string content = expect(TokenType::STRING).value;
    expect(TokenType::CLOSE_PAREN);
    expect(TokenType::SEMICOLON);
    return std::make_unique<PrintExpr>(content);
  }

  std::unique_ptr<Expr> parseStruct() {
    expect(TokenType::STRUCT);
    auto tok = expect(TokenType::IDENTIFIER);
    std::string name = tok.value;
    auto args = std::vector<std::string>();
    auto argstypes = std::vector<std::unique_ptr<Type>>();

    expect(TokenType::OPEN_CURLY);
    while (current().type != TokenType::CLOSE_CURLY) {
      auto idtok = expect(TokenType::IDENTIFIER);
      args.push_back(idtok.value);

      auto typ = parseType();
      argstypes.push_back(std::move(typ));

      if (current().type == TokenType::COMMA) {
        expect(TokenType::COMMA);
      }
    }
    expect(TokenType::CLOSE_CURLY);

    return std::make_unique<StructDefExpr>(name, std::move(args),
                                           std::move(argstypes));
  }

  std::unique_ptr<Type> parseType() {
    auto tok = current();

    if (tok.type == TokenType::INT) {
      expect(TokenType::INT);
      return std::make_unique<TypeInt>();
    }

    if (tok.type == TokenType::FLOAT) {
      expect(TokenType::FLOAT);
      return std::make_unique<TypeFloat>();
    }

    if (tok.type == TokenType::STRING) {
      expect(TokenType::STRING);
      return std::make_unique<TypeString>();
    }

    if (tok.type == TokenType::BOOL) {
      expect(TokenType::BOOL);
      return std::make_unique<TypeBool>();
    }

    if (tok.type == TokenType::BYTE) {
      expect(TokenType::BYTE);
      return std::make_unique<TypeByte>();
    }

    if (tok.type == TokenType::OPEN_BRACKET) {
      expect(TokenType::OPEN_BRACKET);
      auto size = expect(TokenType::NUMBER);
      expect(TokenType::CLOSE_BRACKET);
      auto typ = parseType();

      return std::make_unique<TypeArray>(*typ);
    }

    throw std::runtime_error("Unexpected type: ```" + current().value + "```");
  }

  std::unique_ptr<Expr> parseFunction() {
    expect(TokenType::FUNCTION); // 'func'
    std::string name = expect(TokenType::IDENTIFIER).value;

    expect(TokenType::OPEN_PAREN);
    std::vector<std::string> args;
    std::vector<Type> argsTypes;
    if (current().type != TokenType::CLOSE_PAREN) {
      do {
        args.push_back(expect(TokenType::IDENTIFIER).value);
        auto typ = parseType();
        argsTypes.push_back(*typ);

        if (current().type == TokenType::COMMA)
          expect(TokenType::COMMA);
        else
          break;
      } while (true);
    }
    expect(TokenType::CLOSE_PAREN);

    auto proto = std::make_unique<ProtoExpr>(name, args, argsTypes);
    auto body = parseBlock();
    return std::make_unique<FuncExpr>(std::move(proto), std::move(body));
  }

  std::unique_ptr<BlockExpr> parseBlock() {
    expect(TokenType::OPEN_CURLY);
    std::vector<std::unique_ptr<Expr>> stmts;
    while (current().type != TokenType::CLOSE_CURLY) {
      stmts.push_back(parseStatement());
    }
    expect(TokenType::CLOSE_CURLY);

    return std::make_unique<BlockExpr>(std::move(stmts));
  }

  std::unique_ptr<Expr> parseDeclaration() {
    expect(TokenType::LET); // 'let'
    std::string name = expect(TokenType::IDENTIFIER).value;
    auto typ = parseType();
    expect(TokenType::ASSIGNMENT);
    auto value = parseExpression();
    expect(TokenType::SEMICOLON);

    return std::make_unique<DeclarationExpr>(name, std::move(value), *typ);
  }

  std::unique_ptr<Expr> parseEquals(std::unique_ptr<Expr> left) {
    expect(TokenType::EQUALS);

    auto right = parseExpression();

    return std::make_unique<EqExpr>(std::move(left), std::move(right));
  }

  std::unique_ptr<Expr> parseLessThan(std::unique_ptr<Expr> left) {
    expect(TokenType::LESS_THAN);

    auto right = parseExpression();

    return std::make_unique<LTExpr>(std::move(left), std::move(right));
  }

  std::unique_ptr<Expr> parseLessThanEquals(std::unique_ptr<Expr> left) {
    expect(TokenType::LESS_THAN_EQUALS);

    auto right = parseExpression();

    return std::make_unique<LTEExpr>(std::move(left), std::move(right));
  }

  std::unique_ptr<Expr> parseGreaterThanEquals(std::unique_ptr<Expr> left) {
    expect(TokenType::GREATER_THAN_EQUALS);

    auto right = parseExpression();

    return std::make_unique<GTEExpr>(std::move(left), std::move(right));
  }

  std::unique_ptr<Expr> parseGreaterThan(std::unique_ptr<Expr> left) {
    expect(TokenType::GREATER_THAN);

    auto right = parseExpression();

    return std::make_unique<GTExpr>(std::move(left), std::move(right));
  }

  bool isCompareOp(TokenType t) {
    if (t == TokenType::EQUALS || t == TokenType::GREATER_THAN_EQUALS ||
        t == TokenType::GREATER_THAN || t == TokenType::LESS_THAN_EQUALS ||
        t == TokenType::LESS_THAN) {
      return true;
    }

    return false;
  }

  std::unique_ptr<Expr> parseExpression(std::unique_ptr<Expr> left = nullptr) {
    trace("begin parse expression, current token: " + current().value + "\n");
    if (current().type == TokenType::EQUALS) {
      trace("redirect to parse =\n");
      return parseEquals(std::move(left));
    }

    if (current().type == TokenType::LESS_THAN) {
      trace("redirect to parse <\n");
      return parseLessThan(std::move(left));
    }

    if (current().type == TokenType::LESS_THAN_EQUALS) {
      trace("redirect to parse <=\n");
      return parseLessThanEquals(std::move(left));
    }

    if (current().type == TokenType::GREATER_THAN) {
      trace("redirect to parse >\n");
      return parseGreaterThan(std::move(left));
    }

    if (current().type == TokenType::GREATER_THAN_EQUALS) {
      trace("redirect to parse >=\n");
      return parseGreaterThanEquals(std::move(left));
    }

    if (current().type == TokenType::NUMBER) {
      trace("parsing number \n");
      auto expr = NumberExpr(std::stoi(expect(TokenType::NUMBER).value));
      auto num = std::make_unique<Expr>(expr);

      if (isCompareOp(current().type)) {
        return parseExpression(std::move(num));
      }

      return num;
    }

    if (current().type == TokenType::IDENTIFIER) {
      trace("parsing identifier \n");
      auto id = IdExpr(expect(TokenType::IDENTIFIER).value);
      auto exp = std::make_unique<IdExpr>(id);

      if (isCompareOp(current().type)) {
        return parseExpression(std::move(exp));
      }

      return exp;
    }

    if (current().type == TokenType::END_OF_FILE) {
      return nullptr;
    }

    throw std::runtime_error("Unexpected token `" + current().value + "`");

    return nullptr;
  }
};
