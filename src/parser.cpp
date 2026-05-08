#include <algorithm>
#include <iostream>
#include <memory>
#include <parser/parser.h>
#include <stdexcept>
#include <utility>
#include <vector>

Token Parser::current() {
  if (pos >= tokens.size()) {
    return Token{TokenType::END_OF_FILE, ""};
  }

  return tokens[pos];
}

Token Parser::expect(TokenType type) {
  if (isAtEnd()) {
    throw std::runtime_error("Unexpected end of file. Expected: " +
                             std::to_string((int)type) + " " + current().value);
  }

  if (current().type == type) {
    return tokens[pos++];
  }

  throw std::runtime_error("Unexpected token: ```" + current().value + "```");
}

bool Parser::isAtEnd() {
  return pos >= tokens.size() || current().type == TokenType::END_OF_FILE;
}

std::unique_ptr<BlockExpr> Parser::parseProgram() {
  std::vector<std::unique_ptr<Expr>> stmts;
  while (!isAtEnd()) {
    stmts.push_back(parseStatement());
  }
  return std::make_unique<BlockExpr>(std::move(stmts));
}

void Parser::trace(std::string msg) {
  return;
  std::cout << "PARSER TRACE " << msg;
}

std::unique_ptr<Expr> Parser::parseStatement() {
  trace("begin parse statement\n");

  if (current().type == TokenType::FUNCTION ||
      current().type == TokenType::MAIN)
    return parseFunction();
  if (current().type == TokenType::STRUCT)
    return parseStruct();
  if (current().type == TokenType::PRINT_F)
    return parsePrintFormatted();
  if (current().type == TokenType::PRINT)
    return parsePrint();
  if (current().type == TokenType::LET)
    return parseDeclaration();
  if (current().type == TokenType::RETURN)
    return parseReturn();
  if (current().type == TokenType::DIALECT) {
    parseDialect();
    return parseStatement();
  }
  if (current().type == TokenType::IF)
    return parseIf();

  trace("end parse statement\n");

  return parseExpression();
}

int Parser::getPrecedence(TokenType type) {
  switch (type) {
  case TokenType::EQUALS:
  case TokenType::LESS_THAN:
  case TokenType::LESS_THAN_EQUALS:
  case TokenType::GREATER_THAN:
  case TokenType::GREATER_THAN_EQUALS:
    return 10; // Comparison
  case TokenType::PLUS:
  case TokenType::MINUS:
    return 20; // Addition/Subtraction
  case TokenType::MULTIPLY:
  case TokenType::DIVIDE:
    return 30; // Multiplication/Division
  default:
    return -1; // Not an operator
  }
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (current().type == TokenType::TRUE) {
    std::string name = expect(TokenType::TRUE).value;
    return std::make_unique<BoolExpr>(true);
  }
  if (current().type == TokenType::FALSE) {
    std::string name = expect(TokenType::FALSE).value;
    return std::make_unique<BoolExpr>(false);
  }
  if (current().type == TokenType::NUMBER) {
    int val = std::stoi(expect(TokenType::NUMBER).value);
    return std::make_unique<NumberExpr>(val);
  }
  if (current().type == TokenType::IDENTIFIER) {
    std::string name = expect(TokenType::IDENTIFIER).value;
    return std::make_unique<IdExpr>(name);
  }
  if (current().type == TokenType::STRING) {
    std::string name = expect(TokenType::STRING).value;
    return std::make_unique<StrExpr>(name);
  }
  if (current().type == TokenType::OPEN_PAREN) {
    expect(TokenType::OPEN_PAREN);
    auto expr = parseExpression(0); // Recursively parse inner expr
    expect(TokenType::CLOSE_PAREN);
    return expr;
  }

  if (isAtEnd())
    return nullptr;

  throw std::runtime_error("Expected expression but got " + current().value);
}

void Parser::parseDialect() {
  expect(TokenType::DIALECT);
  expect(TokenType::COLON);
  expect(TokenType::IDENTIFIER);
  expect(TokenType::SEMICOLON);
}

std::unique_ptr<Expr> Parser::parseIf() {
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

  return std::make_unique<IfExpr>(std::move(cond), std::move(b), std::move(e));
}

std::unique_ptr<Expr> Parser::parseReturn() {
  expect(TokenType::RETURN); // 'print'

  auto expr = parseExpression();

  expect(TokenType::SEMICOLON);

  return std::make_unique<ReturnExpr>(std::move(expr));
}
std::unique_ptr<Expr> Parser::parsePrintFormatted() {
  expect(TokenType::PRINT_F);
  expect(TokenType::OPEN_PAREN);
  std::vector<std::unique_ptr<Expr>> values;

  while (current().type != TokenType::CLOSE_PAREN) {
    auto v = parseExpression();
    values.push_back(std::move(v));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    }
  }

  expect(TokenType::CLOSE_PAREN);
  expect(TokenType::SEMICOLON);

  if (values.size() == 0) {
    throw std::runtime_error(
        "formatted print expr should have at least one value");
  }

  auto format = dynamic_cast<StrExpr *>(values[0].get());
  if (format == nullptr) {
    throw std::runtime_error(
        "First arg to formatted print expr should be a string");
  }

  return std::make_unique<Formatted_Print_Expr>(std::move(values));
}

std::unique_ptr<Expr> Parser::parsePrint() {
  expect(TokenType::PRINT); // 'print'
  expect(TokenType::OPEN_PAREN);
  std::vector<std::unique_ptr<Expr>> values;

  while (current().type != TokenType::CLOSE_PAREN) {
    auto v = parseExpression();
    values.push_back(std::move(v));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    }
  }

  expect(TokenType::CLOSE_PAREN);
  expect(TokenType::SEMICOLON);

  return std::make_unique<PrintExpr>(std::move(values));
}

std::unique_ptr<Expr> Parser::parseStruct() {
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
std::unique_ptr<Type> Parser::parseType() {
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

std::unique_ptr<Expr> Parser::parseFunction() {
  trace("begin parse function, current token: " + current().value + "\n");

  std::string name;

  if (current().type == TokenType::MAIN) {
    expect(TokenType::MAIN);
    name = "main";
  } else {
    expect(TokenType::FUNCTION);
    name = expect(TokenType::IDENTIFIER).value;
  }

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

  auto ret = parseType();

  auto proto = std::make_unique<ProtoExpr>(name, args, argsTypes, *ret);
  auto body = parseBlock();
  return std::make_unique<FuncExpr>(std::move(proto), std::move(body));
}

std::unique_ptr<BlockExpr> Parser::parseBlock() {
  expect(TokenType::OPEN_CURLY);
  std::vector<std::unique_ptr<Expr>> stmts;
  while (current().type != TokenType::CLOSE_CURLY) {
    stmts.push_back(parseStatement());
  }
  expect(TokenType::CLOSE_CURLY);

  return std::make_unique<BlockExpr>(std::move(stmts));
}

std::unique_ptr<Expr> Parser::parseDeclaration() {
  expect(TokenType::LET); // 'let'
  std::string name = expect(TokenType::IDENTIFIER).value;
  auto typ = parseType();
  expect(TokenType::ASSIGNMENT);
  auto value = parseExpression();
  expect(TokenType::SEMICOLON);

  return std::make_unique<DeclarationExpr>(name, std::move(value), *typ);
}

std::unique_ptr<Expr> Parser::createBinaryNode(TokenType op,
                                               std::unique_ptr<Expr> left,
                                               std::unique_ptr<Expr> right) {
  switch (op) {
  case TokenType::PLUS:
    return std::make_unique<AddExpr>(std::move(left), std::move(right));
  case TokenType::MINUS:
    return std::make_unique<SubExpr>(std::move(left), std::move(right));
  case TokenType::MULTIPLY:
    return std::make_unique<MulExpr>(std::move(left), std::move(right));
  case TokenType::DIVIDE:
    return std::make_unique<DivExpr>(std::move(left), std::move(right));
  case TokenType::EQUALS:
    return std::make_unique<EqExpr>(std::move(left), std::move(right));
  case TokenType::LESS_THAN:
    return std::make_unique<LTExpr>(std::move(left), std::move(right));
  case TokenType::LESS_THAN_EQUALS:
    return std::make_unique<LTEExpr>(std::move(left), std::move(right));
  case TokenType::GREATER_THAN:
    return std::make_unique<GTExpr>(std::move(left), std::move(right));
  case TokenType::GREATER_THAN_EQUALS:
    return std::make_unique<GTEExpr>(std::move(left), std::move(right));
  default:
    throw std::runtime_error("Unknown operator in expression `" +
                             current().value + "`");
  }
}

std::unique_ptr<Expr> Parser::parseExpression(int minPrecedence) {
  trace("begin parse expression, current token: " + current().value + "\n");
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  switch (current().type) {
  case TokenType::MINUS:
    expect(TokenType::MINUS);

    right = parseExpression(40);
    left = std::make_unique<UnaryMinusExpr>(std::move(right));
    break;
  case TokenType::NOT:
    right = parseExpression(40);
    left = std::make_unique<UnaryNotExpr>(std::move(right));
    break;
  default:
    left = parsePrimary();
  }

  while (true) {
    TokenType opType = current().type;
    int precedence = getPrecedence(opType);

    if (precedence < minPrecedence) {
      break;
    }

    expect(opType);

    auto right = parseExpression(precedence + 1);

    left = createBinaryNode(opType, std::move(left), std::move(right));
  }

  return left;
}
