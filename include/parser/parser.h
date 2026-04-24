#include <ast/node.hpp>
#include <lexer/lexer.hpp>

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
  std::unique_ptr<Expr> parseStatement() {
    if (current().type == TokenType::FUNCTION)
      return parseFunction();
    if (current().type == TokenType::STRUCT)
      return parseStruct();
    if (current().type == TokenType::PRINT)
      return parsePrint();
    if (current().type == TokenType::LET)
      return parseDeclaration();

    return parseExpression();
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

    expect(TokenType::OPEN_CURLY);
    while (current().type != TokenType::CLOSE_CURLY) {
      auto idtok = expect(TokenType::IDENTIFIER);
      auto tytok = expect(TokenType::INT);

      if (current().type == TokenType::COMMA) {
        expect(TokenType::COMMA);
      }
    }
    expect(TokenType::CLOSE_CURLY);

    return std::make_unique<StructDefExpr>(name);
  }

  std::unique_ptr<Type> parseType() {
    auto tok = current();

    if (tok.type == TokenType::INT) {
      expect(TokenType::INT);
      return std::make_unique<TypeInt>();
    }

    throw std::runtime_error("Unexpected type: ```" + current().value + "```");
  }

  std::unique_ptr<Expr> parseFunction() {
    expect(TokenType::FUNCTION); // 'func'
    std::string name = expect(TokenType::IDENTIFIER).value;

    expect(TokenType::OPEN_PAREN);
    std::vector<std::string> args;
    if (current().type != TokenType::CLOSE_PAREN) {
      do {
        args.push_back(expect(TokenType::IDENTIFIER).value);
        if (current().type == TokenType::COMMA)
          expect(TokenType::COMMA);
        else
          break;
      } while (true);
    }
    expect(TokenType::CLOSE_PAREN);

    auto proto = std::make_unique<ProtoExpr>(name, args);
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
    expect(TokenType::ASSIGNMENT);
    auto value = parseExpression();
    expect(TokenType::SEMICOLON);
    return std::make_unique<DeclarationExpr>(name, std::move(value));
  }

  std::unique_ptr<Expr> parseExpression() {
    if (current().type == TokenType::NUMBER) {
      return std::make_unique<NumberExpr>(
          std::stoi(expect(TokenType::NUMBER).value));
    }

    // Handle variables or logic...
    return nullptr;
  }
};
