#include "ast/node.h"
#include "ast/type.h"
#include "lexer/lexer.h"
#include "lexer/tokentype.h"
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

Token Parser::previous() { return tokens[pos - 1]; }
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
Token Parser::expect(
    TokenType type,
    std::function<ParserException(Span errorSpan)> errorHandler) {
  if (isAtEnd()) {
    throw std::runtime_error("Unexpected end of file. Expected: " +
                             std::to_string((int)type) + " " + current().value);
  }

  if (current().type == type) {
    return tokens[pos++];
  }

  throw errorHandler(previous().span);
}

bool Parser::isAtEnd() {
  return pos >= tokens.size() || current().type == TokenType::END_OF_FILE;
}

std::unique_ptr<BlockExpr> Parser::parseProgram() {
  std::vector<std::unique_ptr<Expr>> stmts;
  while (!isAtEnd()) {
    auto stmt = parseStatement();

    if (stmt != nullptr) {
      stmts.push_back(std::move(stmt));
    }
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
    return nullptr;
  }
  if (current().type == TokenType::IF) {
    return parseIf();
  }
  if (current().type == TokenType::TEST) {
    parseTest();
    return nullptr;
  }
  if (current().type == TokenType::TEST_ASSERT_TRUE ||
      current().type == TokenType::TEST_ASSERT_FALSE ||
      current().type == TokenType::TEST_ASSERT_EQUAL ||
      current().type == TokenType::TEST_ASSERT_NOT_EQUAL ||
      current().type == TokenType::TEST_ASSERT_LESS_THAN ||
      current().type == TokenType::TEST_ASSERT_GREATER_THAN ||
      current().type == TokenType::TEST_ASSERT_LESS_THAN_OR_EQUALS ||
      current().type == TokenType::TEST_ASSERT_GREATER_THAN_OR_EQUALS) {
    return parseAsserts();
  }

  trace("end parse statement\n");

  return parseExpression();
}

int Parser::getPrecedence(TokenType type) {
  switch (type) {
  case TokenType::OR:
    return 5;
  case TokenType::AND:
    return 10;
  case TokenType::EQUALS:
  case TokenType::LESS_THAN:
  case TokenType::LESS_THAN_EQUALS:
  case TokenType::GREATER_THAN:
  case TokenType::GREATER_THAN_EQUALS:
    return 10;
  case TokenType::PLUS:
  case TokenType::MINUS:
    return 20;
  case TokenType::MULTIPLY:
  case TokenType::DIVIDE:
    return 30;
  case TokenType::OPEN_PAREN:
    return 40;
  default:
    return -1;
  }
}

std::unique_ptr<Expr> Parser::parseAsserts() {
  switch (current().type) {
  case TokenType::TEST_ASSERT_TRUE: {
    auto span = expect(TokenType::TEST_ASSERT_TRUE).span;
    auto expr = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_True_Expr>(std::move(expr), span);
  }
  case TokenType::TEST_ASSERT_FALSE: {
    auto span = expect(TokenType::TEST_ASSERT_FALSE).span;
    auto expr = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_False_Expr>(std::move(expr), span);
  }

  case TokenType::TEST_ASSERT_EQUAL: {
    auto span = expect(TokenType::TEST_ASSERT_EQUAL).span;
    auto left = parseExpression();
    expect(TokenType::COMMA);
    auto right = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_Equal_Expr>(std::move(left),
                                               std::move(right), span);
  }

  case TokenType::TEST_ASSERT_NOT_EQUAL: {
    auto span = expect(TokenType::TEST_ASSERT_NOT_EQUAL).span;
    auto left = parseExpression();
    expect(TokenType::COMMA);
    auto right = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_Not_Equal_Expr>(std::move(left),
                                                   std::move(right), span);
  }

  case TokenType::TEST_ASSERT_GREATER_THAN: {
    auto span = expect(TokenType::TEST_ASSERT_GREATER_THAN).span;
    auto left = parseExpression();
    expect(TokenType::COMMA);
    auto right = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_Greater_Than_Expr>(std::move(left),
                                                      std::move(right), span);
  }

  case TokenType::TEST_ASSERT_GREATER_THAN_OR_EQUALS: {
    auto span = expect(TokenType::TEST_ASSERT_GREATER_THAN_OR_EQUALS).span;
    auto left = parseExpression();
    expect(TokenType::COMMA);
    auto right = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_Greater_Than_Equals_Expr>(
        std::move(left), std::move(right), span);
  }

  case TokenType::TEST_ASSERT_LESS_THAN: {
    auto span = expect(TokenType::TEST_ASSERT_LESS_THAN).span;
    auto left = parseExpression();
    expect(TokenType::COMMA);
    auto right = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_Less_Than_Expr>(std::move(left),
                                                   std::move(right), span);
  }

  case TokenType::TEST_ASSERT_LESS_THAN_OR_EQUALS: {
    auto span = expect(TokenType::TEST_ASSERT_LESS_THAN_OR_EQUALS).span;
    auto left = parseExpression();
    expect(TokenType::COMMA);
    auto right = parseExpression();
    auto lasttok = expect(TokenType::SEMICOLON);
    span.end = lasttok.span.end;

    return std::make_unique<Assert_Less_Than_Equal_Expr>(
        std::move(left), std::move(right), span);
  }

  default:
    throw std::runtime_error("unknown assertion for token ");
  }
  throw std::runtime_error("unknown assertion for token ");
}

void Parser::parseTest() {
  auto span = expect(TokenType::TEST).span;
  auto name = expect(TokenType::STRING).value;
  auto body = parseBlock();
  span.end = body->span.end;

  auto t = std::make_unique<Test_Expr>(name, std::move(body), span);

  tests.push_back(std::move(t));
}

std::unique_ptr<Expr> Parser::parseFunctionCall(std::unique_ptr<Expr> callee) {
  auto tok = expect(TokenType::OPEN_PAREN);
  auto span = tok.span;

  std::vector<std::unique_ptr<Expr>> args;

  while (current().type != TokenType::CLOSE_PAREN && !isAtEnd()) {
    if (current().type == TokenType::SEMICOLON ||
        current().type == TokenType::CLOSE_CURLY) {
      break;
    }
    auto arg = parseExpression(0);
    args.push_back(std::move(arg));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    }
  }

  auto lasttok = expect(TokenType::CLOSE_PAREN, [tok](Span s) {
    return ParserException(
        "unclosed function call parameter block", s,
        "expected a closing ')' to match the open parenthesis",
        "add a closing parenthesis to terminate the call parameters list");
  });
  span.end = lasttok.span.end;

  return std::make_unique<CallExpr>(std::move(callee), std::move(args), span);
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  if (current().type == TokenType::TRUE) {
    auto tok = expect(TokenType::TRUE);
    return std::make_unique<BoolExpr>(true, tok.span);
  }
  if (current().type == TokenType::FALSE) {
    auto tok = expect(TokenType::FALSE);
    return std::make_unique<BoolExpr>(false, tok.span);
  }
  if (current().type == TokenType::NUMBER) {
    auto tok = expect(TokenType::NUMBER);
    int val = std::stoi(tok.value);
    return std::make_unique<NumberExpr>(val, tok.span);
  }
  if (current().type == TokenType::IDENTIFIER) {
    auto tok = expect(TokenType::IDENTIFIER);
    auto expr = std::make_unique<IdExpr>(tok.value, tok.span);
    return expr;
  }
  if (current().type == TokenType::STRING) {
    auto tok = expect(TokenType::STRING);
    return std::make_unique<StrExpr>(tok.value, tok.span);
  }
  if (current().type == TokenType::OPEN_PAREN) {
    auto first = expect(TokenType::OPEN_PAREN).span;
    auto expr = parseExpression(0);
    first.end = expect(TokenType::CLOSE_PAREN).span.end;
    return expr;
  }

  if (isAtEnd()) {
    return nullptr;
  }

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
  auto span = expect(TokenType::IF).span;

  trace("begin parse if condition\n");
  auto cond = parseExpression();
  trace("end parse if condition\n");

  auto b = parseBlock();

  trace("end parse if\n");
  span.end = b->span.end;

  std::unique_ptr<BlockExpr> e;
  if (current().type == TokenType::ELSE) {
    trace("begin parse else condition\n");
    expect(TokenType::ELSE);
    e = parseBlock();
    trace("end parse else condition\n");
    span.end = e->span.end;
  }

  return std::make_unique<IfExpr>(std::move(cond), std::move(b), std::move(e),
                                  span);
}

std::unique_ptr<Expr> Parser::parseReturn() {
  auto ret = expect(TokenType::RETURN);
  auto span = ret.span;

  auto expr = parseExpression();

  auto lasttok = expect(TokenType::SEMICOLON, [ret](Span s) {
    return ParserException(
        "return statement must be terminated by a semi colon", s, "",
        "add ; after the return statement");
  });
  span.end = lasttok.span.end;

  return std::make_unique<ReturnExpr>(std::move(expr), span);
}

std::unique_ptr<Expr> Parser::parsePrintFormatted() {
  auto tok = expect(TokenType::PRINT_F);
  auto span = tok.span;
  expect(TokenType::OPEN_PAREN);
  std::vector<std::unique_ptr<Expr>> values;

  while (current().type != TokenType::CLOSE_PAREN && !isAtEnd()) {
    auto v = parseExpression();
    values.push_back(std::move(v));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    }
  }

  expect(TokenType::CLOSE_PAREN);
  auto lasttok = expect(TokenType::SEMICOLON, [tok](Span s) {
    return ParserException(
        "formatted print statement must be terminated by a semi colon", s, "",
        "add ; after the print_f statement");
  });
  span.end = lasttok.span.end;

  if (values.size() == 0) {
    throw std::runtime_error(
        "formatted print expr should have at least one value");
  }

  auto format = dynamic_cast<StrExpr *>(values[0].get());
  if (format == nullptr) {
    throw std::runtime_error(
        "First arg to formatted print expr should be a string");
  }

  return std::make_unique<Formatted_Print_Expr>(std::move(values), span);
}

std::unique_ptr<Expr> Parser::parsePrint() {
  auto tok = expect(TokenType::PRINT);
  auto span = tok.span;
  expect(TokenType::OPEN_PAREN);
  std::vector<std::unique_ptr<Expr>> values;

  while (current().type != TokenType::CLOSE_PAREN && !isAtEnd()) {
    auto v = parseExpression();
    values.push_back(std::move(v));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    }
  }

  expect(TokenType::CLOSE_PAREN);
  auto lasttok = expect(TokenType::SEMICOLON, [tok](Span s) {
    return ParserException("print statement must be terminated by a semi colon",
                           s, "", "add ; after the print statement");
  });
  span.end = lasttok.span.end;

  if (values.size() == 0) {
    throw std::runtime_error(
        "formatted print expr should have at least one value");
  }

  return std::make_unique<PrintExpr>(std::move(values), span);
}

std::unique_ptr<Expr> Parser::parseStruct() {
  auto span = expect(TokenType::STRUCT).span;
  auto tok = expect(TokenType::IDENTIFIER);
  std::string name = tok.value;
  auto args = std::vector<std::string>();
  auto argstypes = std::vector<std::unique_ptr<Type>>();

  expect(TokenType::OPEN_CURLY);
  while (current().type != TokenType::CLOSE_CURLY && !isAtEnd()) {
    auto idtok = expect(TokenType::IDENTIFIER);
    args.push_back(idtok.value);

    auto typ = parseType();
    argstypes.push_back(std::move(typ));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    }
  }
  auto lasttok = expect(TokenType::CLOSE_CURLY);
  span.end = lasttok.span.end;

  return std::make_unique<StructDefExpr>(name, std::move(args),
                                         std::move(argstypes), span);
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
    auto slice = true;
    expect(TokenType::OPEN_BRACKET);
    if (current().type == TokenType::NUMBER) {
      auto size = expect(TokenType::NUMBER);
      slice = false;
    }
    expect(TokenType::CLOSE_BRACKET);
    auto typ = parseType();

    if (slice) {
      return std::make_unique<TypeSlice>(std::move(typ));
    }

    // TODO(pathe) add size
    return std::make_unique<TypeArray>(std::move(typ));
  }

  if (tok.type == TokenType::IDENTIFIER) {
    expect(TokenType::IDENTIFIER);
    return std::make_unique<TypeStruct>(tok.value);
  }

  if (tok.type == TokenType::MULTIPLY) {
    expect(TokenType::MULTIPLY);
    auto t = parseType();

    return std::make_unique<TypePointer>(std::move(t));
  }

  throw ParserException("Unexpected type: ```" + current().value + "```",
                        tok.span);
}

std::unique_ptr<Expr> Parser::parseFunction() {
  trace("begin parse function, current token: " + current().value + "\n");

  std::string name;
  bool is_main = false;
  Span span;

  if (current().type == TokenType::MAIN) {
    span = expect(TokenType::MAIN).span;
    name = "main";
    is_main = true;
  } else {
    span = expect(TokenType::FUNCTION).span;
    name = expect(TokenType::IDENTIFIER).value;
  }

  expect(TokenType::OPEN_PAREN);
  std::vector<std::string> args;
  std::vector<std::unique_ptr<Type>> argsTypes;
  if (current().type != TokenType::CLOSE_PAREN) {
    do {
      args.push_back(expect(TokenType::IDENTIFIER).value);
      auto typ = parseType();
      argsTypes.push_back(std::move(typ));

      if (current().type == TokenType::COMMA)
        expect(TokenType::COMMA);
      else
        break;
    } while (true);
  }
  auto lasttok = expect(TokenType::CLOSE_PAREN);
  span.end = lasttok.span.end;

  auto ret = parseType();

  if (is_main) {
    auto retval = dynamic_cast<TypeInt *>(ret.get());
    if (retval == nullptr) {
      throw std::runtime_error("return value of main should be TypeInt");
    }

    if (argsTypes.size() > 3) {
      throw std::runtime_error("main should have at most 3 arguments");
    }

    if (argsTypes.size() >= 1) {
      auto arg0 = dynamic_cast<TypeInt *>(argsTypes[0].get());

      if (arg0 == nullptr) {
        throw std::runtime_error("first argument of main should be of TypeInt");
      }
    }

    if (argsTypes.size() >= 2) {
      auto arg1 = dynamic_cast<TypeSlice *>(argsTypes[1].get());

      if (arg1 == nullptr) {
        throw std::runtime_error(
            "second argument of main should be a slice of strings");
      }
    }

    if (argsTypes.size() >= 3) {
      auto arg1 = dynamic_cast<TypeSlice *>(argsTypes[2].get());

      if (arg1 == nullptr) {
        throw std::runtime_error(
            "third argument of main should be a slice of strings");
      }
    }
  }

  auto proto = std::make_unique<ProtoExpr>(name, args, std::move(argsTypes),
                                           std::move(ret));
  auto body = parseBlock();
  span.end = body->span.end;
  return std::make_unique<FuncExpr>(std::move(proto), std::move(body), span);
}

std::unique_ptr<BlockExpr> Parser::parseBlock() {
  auto span = expect(TokenType::OPEN_CURLY).span;
  std::vector<std::unique_ptr<Expr>> stmts;
  while (current().type != TokenType::CLOSE_CURLY && !isAtEnd()) {
    auto stmt = parseStatement();
    if (stmt != nullptr) {
      stmts.push_back(std::move(stmt));
    }
  }
  // FIXME handle missing close curly
  auto lasttok = expect(TokenType::CLOSE_CURLY);
  span.end = lasttok.span.end;

  return std::make_unique<BlockExpr>(std::move(stmts), span);
}

std::unique_ptr<Expr> Parser::parseDeclaration() {
  auto span = expect(TokenType::LET).span;
  auto name = expect(TokenType::IDENTIFIER);
  auto typ = parseType();
  std::unique_ptr<Expr> value;

  if (current().type != TokenType::SEMICOLON) {
    expect(TokenType::ASSIGNMENT);
    value = parseExpression();
  }

  auto lasttok = expect(TokenType::SEMICOLON, [name](Span s) {
    return ParserException(
        "expected variable declaration to end with a semicolon", s, "", "");
  });
  span.end = lasttok.span.end;

  return std::make_unique<DeclarationExpr>(name.value, std::move(value),
                                           std::move(typ), span);
}

std::unique_ptr<Expr> Parser::createBinaryNode(TokenType op,
                                               std::unique_ptr<Expr> left,
                                               std::unique_ptr<Expr> right) {
  auto span = left->span;
  span.end = right->span.end;
  switch (op) {
  case TokenType::PLUS:
    return std::make_unique<AddExpr>(std::move(left), std::move(right), span);
  case TokenType::MINUS:
    return std::make_unique<SubExpr>(std::move(left), std::move(right), span);
  case TokenType::MULTIPLY:
    return std::make_unique<MulExpr>(std::move(left), std::move(right), span);
  case TokenType::DIVIDE:
    return std::make_unique<DivExpr>(std::move(left), std::move(right), span);
  case TokenType::EQUALS:
    return std::make_unique<EqExpr>(std::move(left), std::move(right), span);
  case TokenType::LESS_THAN:
    return std::make_unique<LTExpr>(std::move(left), std::move(right), span);
  case TokenType::LESS_THAN_EQUALS:
    return std::make_unique<LTEExpr>(std::move(left), std::move(right), span);
  case TokenType::GREATER_THAN:
    return std::make_unique<GTExpr>(std::move(left), std::move(right), span);
  case TokenType::GREATER_THAN_EQUALS:
    return std::make_unique<GTEExpr>(std::move(left), std::move(right), span);
  case TokenType::AND:
    return std::make_unique<Logical_And_Expr>(std::move(left), std::move(right),
                                              span);
  case TokenType::OR:
    return std::make_unique<Logical_Or_Expr>(std::move(left), std::move(right),
                                             span);
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
  case TokenType::MINUS: {
    auto span = expect(TokenType::MINUS).span;

    right = parseExpression(40);
    span.end = right->span.end;
    left = std::make_unique<UnaryMinusExpr>(std::move(right), span);
    break;
  }
  case TokenType::NOT: {
    auto span = expect(TokenType::NOT).span;
    right = parseExpression(40);
    span.end = right->span.end;
    left = std::make_unique<UnaryNotExpr>(std::move(right), span);
    break;
  }
  default:
    left = parsePrimary();
  }

  while (true) {
    TokenType opType = current().type;

    if (opType == TokenType::SEMICOLON || opType == TokenType::OPEN_CURLY ||
        opType == TokenType::CLOSE_PAREN || opType == TokenType::COMMA) {
      break;
    }

    int precedence = getPrecedence(opType);

    if (precedence < minPrecedence) {
      break;
    }

    if (opType == TokenType::OPEN_PAREN) {
      left = parseFunctionCall(std::move(left));
      continue;
    }

    expect(opType);

    auto right = parseExpression(precedence + 1);

    left = createBinaryNode(opType, std::move(left), std::move(right));
  }

  return left;
}

std::vector<std::unique_ptr<Test_Expr>> Parser::Tests() {
  return std::move(tests);
}
