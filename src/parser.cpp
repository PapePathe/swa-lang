#include "ast/node.h"
#include "ast/type.h"
#include "lexer/lexer.h"
#include "lexer/tokentype.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <parser/parser.h>
#include <stdexcept>
#include <utility>
#include <vector>

Token Parser::current() {
  if (pos >= tokens.size()) {
    return Token{TokenType::END_OF_FILE, "", {0}, {0}};
  }

  return tokens[pos];
}

Token Parser::previous() { return tokens[pos - 1]; }
Token Parser::expect(TokenType type) {
  if (isAtEnd()) {
    throw ParserException("Unexpected end of file. Expected: " +
                              std::to_string((int)type) + " " + current().value,
                          current().span);
  }

  if (current().type == type) {
    return tokens[pos++];
  }

  throw ParserException("Unexpected token: ```" + current().value + "```",
                        current().span);
}
Token Parser::expect(
    TokenType type,
    std::function<ParserException(Span errorSpan)> errorHandler) {
  if (isAtEnd()) {
    throw ParserException("Unexpected end of file. Expected: " +
                              std::to_string((int)type) + " " + current().value,
                          current().span);
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
  Span programSpan;
  programSpan.start = tokens.front().span.start;

  while (!isAtEnd()) {
    auto stmt = parseStatement();

    if (stmt != nullptr) {
      stmts.push_back(std::move(stmt));
    }
  }
  programSpan.end = previous().span.end;

  return std::make_unique<BlockExpr>(std::move(stmts), programSpan);
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

  if (current().type == TokenType::OPEN_CURLY) {
    return parseBlock();
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
  case TokenType::OPEN_BRACKET:
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
    throw ParserException("unknown assertion for token ", current().span);
  }
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
  auto span = callee->span;
  auto tok = expect(TokenType::OPEN_PAREN);

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
  std::unique_ptr<Expr> expr;
  if (current().type == TokenType::TRUE) {
    auto tok = expect(TokenType::TRUE);
    expr = std::make_unique<BoolExpr>(true, tok.span);

  } else if (current().type == TokenType::FALSE) {
    auto tok = expect(TokenType::FALSE);
    expr = std::make_unique<BoolExpr>(false, tok.span);

  } else if (current().type == TokenType::NUMBER) {
    auto tok = expect(TokenType::NUMBER);
    expr = std::make_unique<NumberExpr>(std::stoi(tok.value), tok.span);

  } else if (current().type == TokenType::NUMBER_FLOAT) {
    auto tok = expect(TokenType::NUMBER_FLOAT);
    expr = std::make_unique<FloatExpr>(std::stod(tok.value),
                                       FloatPrecision::F32, tok.span);
  } else if (current().type == TokenType::IDENTIFIER) {
    auto tok = expect(TokenType::IDENTIFIER);
    expr = std::make_unique<IdExpr>(tok.value, tok.span);

  } else if (current().type == TokenType::STRING) {
    auto tok = expect(TokenType::STRING);
    expr = std::make_unique<StrExpr>(tok.value, tok.span);

  } else if (current().type == TokenType::OPEN_PAREN) {
    auto span = expect(TokenType::OPEN_PAREN).span;
    expr = parseExpression(0);
    span.end = expect(TokenType::CLOSE_PAREN).span.end;
    expr->span = span;

  } else if (current().type == TokenType::OPEN_BRACKET) {
    return parseArrayInitExpr();
  }

  if (!expr) {
    throw ParserException("Expected expression but got " + current().value,
                          current().span, "", "");
  }

  return expr;
}

std::unique_ptr<Expr> Parser::parseArrayAccess(std::unique_ptr<Expr> sequence) {
  auto openTok = expect(TokenType::OPEN_BRACKET);
  Span s = openTok.span;

  auto indexExpr = parseExpression(0);

  auto lasttok = expect(TokenType::CLOSE_BRACKET, [openTok](Span s) {
    return ParserException(
        "Missing closing bracket in array subscript access site", s,
        "Expected a matching ']' to finish indexing calculations",
        "Add a closing bracket ']'");
  });

  s.end = lasttok.span.end;

  return std::make_unique<Array_Access_Expr>(std::move(sequence),
                                             std::move(indexExpr), s);
}

std::unique_ptr<Expr> Parser::parseArrayInitExpr() {
  auto span = expect(TokenType::OPEN_BRACKET).span;
  std::vector<std::unique_ptr<Expr>> elements;

  while (current().type != TokenType::CLOSE_BRACKET && !isAtEnd()) {
    auto element = parseExpression(0);
    elements.push_back(std::move(element));

    if (current().type == TokenType::COMMA) {
      expect(TokenType::COMMA);
    } else if (current().type != TokenType::CLOSE_BRACKET) {
      throw ParserException("Expected comma or ']' in array initializer",
                            current().span);
    }
  }

  auto lasttok = expect(TokenType::CLOSE_BRACKET);
  span.end = lasttok.span.end;

  return std::make_unique<Array_Init_Expr>(std::move(elements), span);
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
    throw ParserException("formatted print expr should have at least one value",
                          span);
  }

  auto format = dynamic_cast<StrExpr *>(values[0].get());
  if (format == nullptr) {
    throw ParserException(
        "First arg to formatted print expr should be a string", span);
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
    throw ParserException("print expr should have at least one value", span);
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
    auto span = expect(TokenType::INT).span;
    return std::make_unique<TypeInt>(span);
  }

  if (tok.type == TokenType::FLOAT) {
    auto span = expect(TokenType::FLOAT).span;
    return std::make_unique<TypeFloat>(span);
  }

  if (tok.type == TokenType::STRING) {
    auto span = expect(TokenType::STRING).span;
    return std::make_unique<TypeString>(span);
  }

  if (tok.type == TokenType::BOOL) {
    auto span = expect(TokenType::BOOL).span;
    return std::make_unique<TypeBool>(span);
  }

  if (tok.type == TokenType::BYTE) {
    auto span = expect(TokenType::BYTE).span;
    return std::make_unique<TypeByte>(span);
  }

  if (tok.type == TokenType::OPEN_BRACKET) {
    auto slice = true;
    auto span = expect(TokenType::OPEN_BRACKET).span;
    std::string size;
    if (current().type == TokenType::NUMBER) {
      size = expect(TokenType::NUMBER).value;
      slice = false;
    }
    expect(TokenType::CLOSE_BRACKET, [span](Span s) {
      auto msg = "expected close bracket in array or slice";
      return ParserException(msg, s, "", "");
    });
    auto typ = parseType();

    span.end = typ->span.end;

    if (slice) {
      return std::make_unique<TypeSlice>(std::move(typ), span);
    }

    return std::make_unique<TypeArray>(std::stoi(size), std::move(typ), span);
  }

  if (tok.type == TokenType::IDENTIFIER) {
    expect(TokenType::IDENTIFIER);
    return std::make_unique<TypeStruct>(tok.value, tok.span);
  }

  if (tok.type == TokenType::MULTIPLY) {
    auto span = expect(TokenType::MULTIPLY).span;
    auto t = parseType();
    span.end = t->span.end;

    return std::make_unique<TypePointer>(std::move(t), span);
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
  auto ret = parseType();
  span.end = ret->span.end;

  auto proto = std::make_unique<ProtoExpr>(name, args, std::move(argsTypes),
                                           std::move(ret), span);
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
  Span span;
  span.start = left->span.start;
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
    throw ParserException("Unknown operator in expression `" + current().value +
                              "`",
                          current().span);
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

    if (opType == TokenType::OPEN_BRACKET) {
      left = parseArrayAccess(std::move(left));
      continue;
    }

    expect(opType);

    auto right = parseExpression(precedence + 1);

    left = createBinaryNode(opType, std::move(left), std::move(right));
    if (right) {
      left->span.end.offset = right->span.end.offset;
    }
  }

  if (!left) {
    throw ParserException("left is a nullptr " + current().value,
                          current().span);
  }

  return left;
}

std::vector<std::unique_ptr<Test_Expr>> Parser::Tests() {
  return std::move(tests);
}
