#include "ast/node.h"
#include "lexer/lexer.h"
#include "parser/exception.h"
#include <cstddef>
#include <exception>
#include <gtest/gtest.h>
#include <iostream>
#include <lexer/keywords.h>
#include <lexer/tokentypestring.h>
#include <memory>
#include <parser/parser.h>
#include <stdexcept>
#include <utility>
#include <vector>

std::vector<Token> getTokens(const std::string &source) {
  Lexer lexer(source, KEYWORDS_ENGLISH);
  return lexer.tokenize();
}

void debugTokens(std::vector<Token> tokens) {
  for (auto tok : tokens) {
    std::cout << "TOKEN " << tok.value << " " << tokenTypeToString(tok.type)
              << "(" << tok.span.start.offset << "," << tok.span.end.offset
              << ")"
              << "\n";
  }
}

class ParserTests : public ::testing::Test {
public:
  void ASSERT_EXPR_NUMBER(Expr *node, int expected) {
    ASSERT_NE(node, nullptr);
    auto *casted = dynamic_cast<NumberExpr *>(node);
    ASSERT_NE(casted, nullptr) << "Node is not a NumberExpr";
    EXPECT_EQ(casted->Value, expected);
  }

  void ASSERT_EXPR_FLOAT(Expr *node, double expected) {
    ASSERT_NE(node, nullptr);
    auto *casted = dynamic_cast<FloatExpr *>(node);
    ASSERT_NE(casted, nullptr) << "Node is not a FloatExpr";
    EXPECT_EQ(casted->Value, expected);
  }

  void ASSERT_EXPR_ID(Expr *node, std::string expected) {
    ASSERT_NE(node, nullptr);
    auto *casted = dynamic_cast<IdExpr *>(node);
    ASSERT_NE(casted, nullptr) << "Node is not a IdExpr";
    EXPECT_EQ(casted->Name, expected);
  }

  void ASSERT_EXPR_STRING(Expr *node, std::string expected) {
    ASSERT_NE(node, nullptr);
    auto *casted = dynamic_cast<StrExpr *>(node);
    ASSERT_NE(casted, nullptr) << "Node is not a StrExpr";
    EXPECT_EQ(casted->Name, expected);
  }

  void ASSERT_EXPR_BOOL(Expr *node, bool expected) {
    ASSERT_NE(node, nullptr);
    auto *casted = dynamic_cast<BoolExpr *>(node);
    ASSERT_NE(casted, nullptr) << "Node is not a BoolExpr";
    EXPECT_EQ(casted->Value, expected);
  }

  AddExpr *ASSERT_EXPR_ADD(Expr *node) {
    EXPECT_NE(node, nullptr);
    auto *casted = dynamic_cast<AddExpr *>(node);
    EXPECT_NE(casted, nullptr) << "Node is not a AddExpr";

    return casted;
  }

  DeclarationExpr *ASSERT_EXPR_DECL(Expr *node, std::string expected) {
    EXPECT_NE(node, nullptr);
    auto *casted = dynamic_cast<DeclarationExpr *>(node);
    EXPECT_NE(casted, nullptr) << "Node is not a DeclarationExpr";
    EXPECT_EQ(casted->Name, expected);

    return casted;
  }

  Array_Init_Expr *ASSERT_EXPR_ARRAY(Expr *node, size_t elementsCount) {
    EXPECT_NE(node, nullptr);
    auto *casted = dynamic_cast<Array_Init_Expr *>(node);
    EXPECT_NE(casted, nullptr) << "Node is not a Array_Init_Expr";
    EXPECT_EQ(casted->Elements.size(), elementsCount);

    return casted;
  }

  Array_Access_Expr *ASSERT_EXPR_ARRAY_ACCESS(Expr *node) {
    EXPECT_NE(node, nullptr);
    auto *casted = dynamic_cast<Array_Access_Expr *>(node);
    EXPECT_NE(casted, nullptr) << "Node is not a Array_Access_Expr";

    return casted;
  }

  std::vector<std::unique_ptr<Expr>> PARSE_PROGRAM(const std::string &input,
                                                   size_t expectedCount) {
    auto tokens = getTokens(input);
    // debugTokens(tokens);
    Parser parser(tokens);
    auto program = parser.parseProgram();

    EXPECT_NE(program, nullptr) << "Parser returned nullptr for: " << input;

    EXPECT_EQ(program->Exprs.size(), expectedCount)
        << "Statement count mismatch for: " << input;

    return std::move(program->Exprs);
  }

  std::pair<std::vector<std::unique_ptr<Expr>>,
            std::vector<std::unique_ptr<Test_Expr>>>
  PARSE_TESTABLE_PROGRAM(const std::string &input, size_t expectedCount) {
    auto tokens = getTokens(input);
    Parser parser(tokens);
    auto program = parser.parseProgram();

    EXPECT_NE(program, nullptr) << "Parser returned nullptr for: " << input;

    EXPECT_EQ(program->Exprs.size(), expectedCount)
        << "Statement count mismatch for: " << input;

    return {std::move(program->Exprs), std::move(parser.Tests())};
  }
  void AssertParserError(const std::string &source,
                         const std::string &expectedMessage,
                         const std::string &expectedLabel,
                         size_t expectedStartOffset, size_t expectedEndOffset) {
    SourceManager sm(source);
    Lexer lexer(source, KEYWORDS_ENGLISH);
    Parser parser(lexer.tokenize());

    try {
      // Force evaluation of the program execution block
      auto ast = parser.parseProgram();
      FAIL() << "Parser unexpectedly succeeded for invalid source: " << source;
    } catch (const ParserException &err) {
      // Verify the primary structural message details
      EXPECT_EQ(err.getMessage(), expectedMessage);

      // Use a getter on your exception to verify the contextual label text
      EXPECT_EQ(err.getLabelText(), expectedLabel);

      // Verify the span points to the exact problem byte range
      Span errorSpan = err.getSpan();
      EXPECT_EQ(errorSpan.start.offset, expectedStartOffset)
          << "Mismatched start offset for error in: " << source;
      EXPECT_EQ(errorSpan.end.offset, expectedEndOffset)
          << "Mismatched end offset for error in: " << source;
    } catch (...) {
      FAIL() << "Parser threw an unexpected exception type instead of "
                "ParserException";
    }
  }
};

TEST_F(ParserTests, ParseMinimalProgram) {
  std::string input = "main()int { print(\"Salam\"); }";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 0);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 0);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
  ASSERT_EQ(node->span.start.offset, 0);
  ASSERT_EQ(node->span.end.offset, 29);
}

TEST_F(ParserTests, ParseMinimalProgramWithCommandArgs) {
  std::string input =
      "main(arguments_count int, arguments []string)int { return 0; }";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 2);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 2);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
  ASSERT_EQ(node->span.start.offset, 0);
  ASSERT_EQ(node->span.end.offset, 62);
}

TEST_F(ParserTests, ParseMinimalProgramWithCommandArgsAndEnv) {
  std::string input = "main(arguments_count int, arguments []string, "
                      "environment []string)int { return 0; }";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 3);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 3);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
}

TEST_F(ParserTests, ParseMinimalProgramWithDialect) {
  std::string input = R"(
    dialect:english;
    main() int {
      print("Salam");
    }
  )";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 0);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 0);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
}

TEST_F(ParserTests, ParseEmptyPrintStatement) {
  std::string input = "print();";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), ParserException);
}

TEST_F(ParserTests, ParseEmptyPrintFStatement) {
  std::string input = "print_f();";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), ParserException);
}

TEST_F(ParserTests, ParseInvalidPrintFStatement) {
  std::string input = "print_f(x);";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), ParserException);
}

TEST_F(ParserTests, ParsePrintFStatement) {
  std::string input = "print_f(\"Hello\");";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto printNode = dynamic_cast<Formatted_Print_Expr *>(stmts[0].get());
  ASSERT_NE(printNode, nullptr);
  ASSERT_EQ(printNode->Values.size(), 1);
  ASSERT_EQ(printNode->span.start.offset, 0);
  ASSERT_EQ(printNode->span.end.offset, 17);

  ASSERT_EXPR_STRING(printNode->Values[0].get(), "Hello");
}

TEST_F(ParserTests, ParsePrintStatement) {
  std::string input = "print(\"Hello\");";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto printNode = dynamic_cast<PrintExpr *>(stmts[0].get());
  ASSERT_NE(printNode, nullptr);
  ASSERT_EQ(printNode->Values.size(), 1);
  ASSERT_EQ(printNode->span.start.offset, 0);
  ASSERT_EQ(printNode->span.end.offset, 15);

  ASSERT_EXPR_STRING(printNode->Values[0].get(), "Hello");
}

TEST_F(ParserTests, ParsePrintStatement2) {
  std::string input = R"( print("10 - 3 - 2 = %d", a);)";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto printNode = dynamic_cast<PrintExpr *>(stmts[0].get());
  ASSERT_NE(printNode, nullptr);
  ASSERT_EQ(printNode->Values.size(), 2);

  ASSERT_EXPR_STRING(printNode->Values[0].get(), "10 - 3 - 2 = %d");
  ASSERT_EXPR_ID(printNode->Values[1].get(), "a");
}

TEST_F(ParserTests, ParsePrintStatement3) {
  std::string input = R"( -5)";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = dynamic_cast<UnaryMinusExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_NUMBER(node->Right.get(), 5);
}

TEST_F(ParserTests, AddExpr) {
  std::string input = "x + 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<AddExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, CompositeBinaryExpr1) {
  std::string input = "2 + 3 * 4";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<AddExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_NUMBER(node->Left.get(), 2);

  auto mul = dynamic_cast<MulExpr *>(node->Right.get());
  ASSERT_NE(mul, nullptr);

  ASSERT_EXPR_NUMBER(mul->Left.get(), 3);
  ASSERT_EXPR_NUMBER(mul->Right.get(), 4);
}

TEST_F(ParserTests, CompositeBinaryExpr2) {
  std::string input = "(2 + 3) * 4";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<MulExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto add = dynamic_cast<AddExpr *>(node->Left.get());
  ASSERT_NE(add, nullptr);
  ASSERT_EXPR_NUMBER(add->Left.get(), 2);
  ASSERT_EXPR_NUMBER(add->Right.get(), 3);
  ASSERT_EXPR_NUMBER(node->Right.get(), 4);
}

TEST_F(ParserTests, SubExpr) {
  std::string input = "x - 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<SubExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, MulExpr) {
  std::string input = "x * 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<MulExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, DivExpr) {
  std::string input = "x / 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<DivExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

// FIXME
// TEST_F(ParserTests, ConditionalNotEq) {
//   std::string input = "!true";
//   auto tokens = getTokens(input);
//   debugTokens(tokens);
//
//   Parser parser(tokens);
//
//   auto program = parser.parseProgram();
//   auto &stmts = program->Exprs;
//   ASSERT_EQ(stmts.size(), 1);
//
//    auto node = dynamic_cast<UnaryNotExpr *>(stmts[0].get());
//    ASSERT_NE(node, nullptr);
//
//    auto right = dynamic_cast<NumberExpr *>(node->Right.get());
//    ASSERT_EQ(right->Value, 0);
// }

TEST_F(ParserTests, ConditionalEq) {
  std::string input = "x = 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<EqExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, ConditionalGt) {
  std::string input = "x > 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<GTExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, ConditionalGte) {
  std::string input = "x >= 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<GTEExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, ConditionalLt) {
  std::string input = "x < 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<LTExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, ConditionalLte) {
  std::string input = "x <= 0 ";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = dynamic_cast<LTEExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  ASSERT_EXPR_ID(node->Left.get(), "x");
  ASSERT_EXPR_NUMBER(node->Right.get(), 0);
}

TEST_F(ParserTests, ParseIf) {
  std::string input = "if x = 0 {print(\"x\");}";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto ifnode = dynamic_cast<IfExpr *>(stmts[0].get());
  ASSERT_NE(ifnode->Success, nullptr);
  ASSERT_EQ(ifnode->Failure, nullptr);
  ASSERT_NE(ifnode->Cond, nullptr);

  auto cond = dynamic_cast<EqExpr *>(ifnode->Cond.get());
  ASSERT_EXPR_ID(cond->Left.get(), "x");
  ASSERT_EXPR_NUMBER(cond->Right.get(), 0);
}

TEST_F(ParserTests, ParseElse) {
  std::string input = "if x = 0 { print(\"x\"); }"
                      "else { print(\"x\"); }";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto ifnode = dynamic_cast<IfExpr *>(stmts[0].get());
  ASSERT_NE(ifnode->Cond, nullptr);
  ASSERT_NE(ifnode->Success, nullptr);
  ASSERT_NE(ifnode->Failure, nullptr);

  auto cond = dynamic_cast<EqExpr *>(ifnode->Cond.get());

  ASSERT_EXPR_ID(cond->Left.get(), "x");
  ASSERT_EXPR_NUMBER(cond->Right.get(), 0);
}

TEST_F(ParserTests, ParseVariableDeclaration) {
  std::string input = "let x int := 42;";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto declNode = dynamic_cast<DeclarationExpr *>(stmts[0].get());
  ASSERT_NE(declNode, nullptr);
  EXPECT_EQ(declNode->Name, "x");

  auto val = dynamic_cast<NumberExpr *>(declNode->Value.get());
  EXPECT_EQ(val->Value, 42);

  // FIXME assert declNode->T == TypeInt
}

TEST_F(ParserTests, ParseFunctionDefinition) {
  std::string input =
      "func my_func(a int, b float, c bool, d string, e byte) int "
      "{ print(\"hi\"); }";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto funcNode = dynamic_cast<FuncExpr *>(stmts[0].get());

  ASSERT_NE(funcNode, nullptr);
  ASSERT_NE(funcNode->Body, nullptr);
  ASSERT_NE(funcNode->Proto, nullptr);
  ASSERT_EQ(funcNode->Proto->Name, "my_func");
  ASSERT_EQ(funcNode->Proto->Args.size(), 5);
  ASSERT_EQ(funcNode->Proto->Args[0], "a");
  ASSERT_EQ(funcNode->Proto->Args[1], "b");
  ASSERT_EQ(funcNode->Proto->Args[2], "c");
  ASSERT_EQ(funcNode->Proto->Args[3], "d");
  ASSERT_EQ(funcNode->Proto->Args[4], "e");
  ASSERT_EQ(funcNode->Proto->ArgsTypes.size(), 5);
}

TEST_F(ParserTests, ParseStructDefinition) {
  std::string input = "struct Point { x int }";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto structNode = dynamic_cast<StructDefExpr *>(stmts[0].get());
  ASSERT_NE(structNode, nullptr);
  ASSERT_EQ(structNode->Name, "Point");
  ASSERT_EQ(structNode->FieldNames.size(), 1);
  ASSERT_EQ(structNode->FieldNames[0], "x");
}

TEST_F(ParserTests, ParseStructDefinitionAllTypes) {
  std::string input =
      "struct Point { x int, y float, a string, b bool, c byte, "
      "d [2]int}";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto structNode = dynamic_cast<StructDefExpr *>(stmts[0].get());
  ASSERT_NE(structNode, nullptr);

  ASSERT_EQ(structNode->FieldTypes.size(), 6);

  ASSERT_EQ(structNode->FieldNames.size(), 6);
  ASSERT_EQ(structNode->FieldNames[0], "x");
  ASSERT_EQ(structNode->FieldNames[1], "y");
  ASSERT_EQ(structNode->FieldNames[2], "a");
  ASSERT_EQ(structNode->FieldNames[3], "b");
  ASSERT_EQ(structNode->FieldNames[4], "c");
  ASSERT_EQ(structNode->FieldNames[5], "d");
  ASSERT_EQ(structNode->Name, "Point");
}

TEST_F(ParserTests, Return) {
  std::string input = "return 0;";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto returnexpr = dynamic_cast<ReturnExpr *>(stmts[0].get());
  ASSERT_NE(returnexpr, nullptr);

  ASSERT_EXPR_NUMBER(returnexpr->Value.get(), 0);
}

// TEST_F(ParserTests, MissingClosingParen) {
//   std::string input = "myFunc(1, 2";
//   auto tokens = getTokens(input);
//   Parser parser(tokens);
//
//   EXPECT_THROW(parser.parseProgram(), std::runtime_error);
// }

TEST_F(ParserTests, TrailingComma) {
  std::string input = "myFunc(1, )";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto call = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 1);
  ASSERT_EXPR_NUMBER(call->Args[0].get(), 1);
}

// FIXME error is not being raised
// TEST_F(ParserTests, EmptyArgInMiddle) {
//  std::string input = "myFunc(1, , 3)";
//  auto tokens = getTokens(input);
//  Parser parser(tokens);
//
//  EXPECT_THROW(parser.parseProgram(), std::runtime_error);
//}

TEST_F(ParserTests, MultipleArguments) {
  std::string input = "add(x, 10, \"result\")";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto call = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 3);

  ASSERT_EXPR_ID(call->Args[0].get(), "x");
  ASSERT_EXPR_NUMBER(call->Args[1].get(), 10);
  ASSERT_EXPR_STRING(call->Args[2].get(), "result");
}

TEST_F(ParserTests, NestedFunctionCalls) {
  std::string input = "highOrderFunc(square(abs(y)))";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto outer = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(outer, nullptr);
  ASSERT_EQ(outer->Args.size(), 1);

  ASSERT_EXPR_ID(outer->Callee.get(), "highOrderFunc");

  auto middle = dynamic_cast<CallExpr *>(outer->Args[0].get());
  ASSERT_NE(middle, nullptr);
  ASSERT_EQ(middle->Args.size(), 1);
  ASSERT_EXPR_ID(middle->Callee.get(), "square");

  auto inner = dynamic_cast<CallExpr *>(middle->Args[0].get());
  ASSERT_NE(inner, nullptr);
  ASSERT_EQ(inner->Args.size(), 1);
  ASSERT_EXPR_ID(inner->Callee.get(), "abs");
}

TEST_F(ParserTests, FunctionCallInArithmetic) {
  std::string input = "5 + getVal() * 2";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto add = dynamic_cast<AddExpr *>(stmts[0].get());
  ASSERT_NE(add, nullptr);

  auto mul = dynamic_cast<MulExpr *>(add->Right.get());
  ASSERT_NE(mul, nullptr);

  auto call = dynamic_cast<CallExpr *>(mul->Left.get());
  ASSERT_NE(call, nullptr);

  ASSERT_EXPR_ID(call->Callee.get(), "getVal");
}

TEST_F(ParserTests, SimpleFunctionCallNoArgs) {
  std::string input = "myFunc()";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto call = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->Args.size(), 0);

  ASSERT_EXPR_ID(call->Callee.get(), "myFunc");
}

TEST_F(ParserTests, FunctionCallWithSingleArg) {
  std::string input = "myFunc(42)";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto call = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->Args.size(), 1);

  ASSERT_EXPR_ID(call->Callee.get(), "myFunc");
  ASSERT_EXPR_NUMBER(call->Args[0].get(), 42);
}

TEST_F(ParserTests, NestedFunctionCall) {
  std::string input = "exponent(calculate(1 + 2))";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto call = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 1);
  ASSERT_EXPR_ID(call->Callee.get(), "exponent");

  auto inner = dynamic_cast<CallExpr *>(call->Args[0].get());
  ASSERT_NE(inner, nullptr);
  ASSERT_EQ(inner->Args.size(), 1);
  ASSERT_EXPR_ID(inner->Callee.get(), "calculate");
}

TEST_F(ParserTests, FunctionCallPrecedenceInLogic) {
  std::string input = "isTrue() && check(5 + 2)";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto and_expr = dynamic_cast<Logical_And_Expr *>(stmts[0].get());
  ASSERT_NE(and_expr, nullptr);

  auto left = dynamic_cast<CallExpr *>(and_expr->Left.get());
  ASSERT_NE(left, nullptr);
  ASSERT_EQ(left->Args.size(), 0);

  ASSERT_EXPR_ID(left->Callee.get(), "isTrue");

  auto right = dynamic_cast<CallExpr *>(and_expr->Right.get());
  ASSERT_NE(right, nullptr);
  ASSERT_EQ(right->Args.size(), 1);

  ASSERT_EXPR_ID(right->Callee.get(), "check");
}

TEST_F(ParserTests, FunctionCallPrecedenceInLogic2) {
  std::string input = "isTrue() || check(5 + 2)";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto and_expr = dynamic_cast<Logical_Or_Expr *>(stmts[0].get());
  ASSERT_NE(and_expr, nullptr);

  auto left = dynamic_cast<CallExpr *>(and_expr->Left.get());
  ASSERT_NE(left, nullptr);
  ASSERT_EQ(left->Args.size(), 0);

  ASSERT_EXPR_ID(left->Callee.get(), "isTrue");

  auto right = dynamic_cast<CallExpr *>(and_expr->Right.get());
  ASSERT_NE(right, nullptr);
  ASSERT_EQ(right->Args.size(), 1);

  ASSERT_EXPR_ID(right->Callee.get(), "check");
}

TEST_F(ParserTests, CallWithinAssignment) {
  std::string input = "let x int := getVal() + 5;";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto decl = dynamic_cast<DeclarationExpr *>(stmts[0].get());
  ASSERT_NE(decl, nullptr);
}

TEST_F(ParserTests, CallsAsArgumentsInMath) {
  std::string input = "calculate(sum(1, 2) * 3, is_valid(x) && true)";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto call = dynamic_cast<CallExpr *>(stmts[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 2);

  // First Arg: sum(1,2) * 3 (MulExpr)
  auto firstArg = dynamic_cast<MulExpr *>(call->Args[0].get());
  ASSERT_NE(firstArg, nullptr);

  auto mleft = dynamic_cast<CallExpr *>(firstArg->Left.get());
  ASSERT_NE(mleft, nullptr);

  ASSERT_EXPR_NUMBER(firstArg->Right.get(), 3);

  auto secondArg = dynamic_cast<Logical_And_Expr *>(call->Args[1].get());
  ASSERT_NE(secondArg, nullptr);

  auto lleft = dynamic_cast<CallExpr *>(secondArg->Left.get());
  ASSERT_NE(lleft, nullptr);

  auto lright = dynamic_cast<BoolExpr *>(secondArg->Right.get());
  ASSERT_NE(lright, nullptr);
  ASSERT_EQ(lright->Value, true);
}

TEST_F(ParserTests, CallInArrayInitialization) {
  std::string input = "let buffer []byte := generate_buffer(size());";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto decl = dynamic_cast<DeclarationExpr *>(stmts[0].get());
  ASSERT_NE(decl, nullptr);

  auto call = dynamic_cast<CallExpr *>(decl->Value.get());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->Args.size(), 1);
}

TEST_F(ParserTests, CallInsideExpressionSemicolonLoophole) {
  std::string input = "let x int := getVal() + 1;";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto decl = dynamic_cast<DeclarationExpr *>(stmts[0].get());
  ASSERT_NE(decl, nullptr);

  auto add = dynamic_cast<AddExpr *>(decl->Value.get());
  ASSERT_NE(add, nullptr);

  auto call = dynamic_cast<CallExpr *>(add->Left.get());
  ASSERT_NE(call, nullptr);

  ASSERT_EXPR_NUMBER(add->Right.get(), 1);
}

TEST_F(ParserTests, TestFramework) {
  std::string input = R"(
    test "test primitives" {
      let result int := add(5, 5);

      assert_true                   result > 5;
      assert_false                  result > 11;
      assert_equal                  result, 10;
      assert_not_equal              result, 11;
      assert_less_than              result, 10;
      assert_greater_than           result, 10;
      assert_less_than_or_equals    result, 11;
      assert_greater_than_or_equals result, 11;
    }
  )";
  auto [stmts, tests] = PARSE_TESTABLE_PROGRAM(input, 0);

  auto texpr = dynamic_cast<Test_Expr *>(tests[0].get());
  ASSERT_NE(texpr, nullptr);
  ASSERT_EQ(texpr->Name, "test primitives");
  ASSERT_EQ(texpr->Body->Exprs.size(), 9);
}

TEST_F(ParserTests, MissingSemiColonAfterVariableDecl) {
  std::string input = "let x int := 10 let";

  AssertParserError(input,
                    "expected variable declaration to end with a semicolon", "",
                    13, 15);
}

TEST_F(ParserTests, Add_Expr_Span) {
  std::string input = R"(2 + "10")";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto decl = dynamic_cast<AddExpr *>(stmts[0].get());
  ASSERT_NE(decl, nullptr);

  EXPECT_EQ(decl->span.start.offset, 0);
  EXPECT_EQ(decl->span.end.offset, 8);
}

TEST_F(ParserTests, Declaration_Expr_Span) {
  std::string input = R"(
dialect:english;
start() int{
  let x int := 2 + "10";
  return 0;
}
  )";

  auto stmts = PARSE_PROGRAM(input, 1);
  auto *func = dynamic_cast<FuncExpr *>(stmts[0].get());
  auto *decl = dynamic_cast<DeclarationExpr *>(func->Body->Exprs[0].get());
  auto *add = dynamic_cast<AddExpr *>(decl->Value.get());

  ASSERT_NE(func, nullptr);
  ASSERT_NE(decl, nullptr);
  ASSERT_NE(add, nullptr);

  EXPECT_EQ(func->span.start.offset, 18);
  EXPECT_EQ(func->span.end.offset, 69);

  EXPECT_EQ(decl->span.start.offset, 33);
  EXPECT_EQ(decl->span.end.offset, 55);

  EXPECT_EQ(add->span.start.offset, 46);
  EXPECT_EQ(add->span.end.offset, 54);
}

TEST_F(ParserTests, Function_Span_Verification) {
  std::string input = R"(
    dialect:english;
    func new_node(value int, left Node, right Node) int {
      return 0;
    }
    start() int {
      return 0;
    }
  )";

  auto stmts = PARSE_PROGRAM(input, 2);

  // 1. Verify Top-Level Function Node "new_node"
  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "new_node");

  // Entire function layout bounds: from 'f' in func to closing '}'
  ASSERT_EQ(node->span.start.offset, 26); // "func ..."
  ASSERT_EQ(node->span.end.offset, 101);  // "... }"

  // 2. Drill Into Parameters & Type Spans (Inside ProtoExpr)
  ASSERT_EQ(node->Proto->Args.size(), 3);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 3);

  // Parameter 1: "value int"
  // "value" identifier span
  // "int" primitive type span
  ASSERT_EQ(node->Proto->ArgsTypes[0]->span.start.offset, 46); // "int"
  ASSERT_EQ(node->Proto->ArgsTypes[0]->span.end.offset, 49);

  // Parameter 2: "left Node"
  // "Node" custom struct type span
  ASSERT_EQ(node->Proto->ArgsTypes[1]->span.start.offset, 56); // "Node"
  ASSERT_EQ(node->Proto->ArgsTypes[1]->span.end.offset, 60);

  // Parameter 3: "right Node"
  // "Node" custom struct type span
  ASSERT_EQ(node->Proto->ArgsTypes[2]->span.start.offset, 68); // "Node"
  ASSERT_EQ(node->Proto->ArgsTypes[2]->span.end.offset, 72);

  // Return Type Annotation: "int" right before the opening brace
  ASSERT_EQ(node->Proto->Ret->span.start.offset, 74); // "int"
  ASSERT_EQ(node->Proto->Ret->span.end.offset, 77);

  // 3. Verify Body Block Spans (BlockExpr)
  auto body = node->Body.get();
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->Exprs.size(), 1);

  // The block spans from the opening '{' to the closing '}'
  ASSERT_EQ(body->span.start.offset, 78); // "{"
  ASSERT_EQ(body->span.end.offset, 101);  // "}"

  // 4. Verify Internal Statement (ReturnExpr)
  auto retStmt = dynamic_cast<ReturnExpr *>(body->Exprs[0].get());
  ASSERT_NE(retStmt, nullptr);

  // The return statement spans from 'r' in return to trailing semicolon ';'
  ASSERT_EQ(retStmt->span.start.offset, 86); // "return 0;"
  ASSERT_EQ(retStmt->span.end.offset, 95);   // ";"

  // 5. Verify Second Function Expression "start" to ensure no lookahead
  // contamination
  auto startNode = dynamic_cast<FuncExpr *>(stmts[1].get());
  ASSERT_NE(startNode, nullptr);
  ASSERT_EQ(startNode->Proto->Name, "main");

  // Enforce true boundary encapsulation for trailing nodes
  ASSERT_EQ(startNode->span.start.offset, 106); // "start() int ..."
  ASSERT_EQ(startNode->span.end.offset, 141);   // "}"
}

TEST_F(ParserTests, Array_Init_Int) {
  std::string input = "let arr [3]int := [1, 2, 3];";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto value = ASSERT_EXPR_ARRAY(node->Value.get(), 3);

  ASSERT_EXPR_NUMBER(value->Elements[0].get(), 1);
  ASSERT_EXPR_NUMBER(value->Elements[1].get(), 2);
  ASSERT_EXPR_NUMBER(value->Elements[2].get(), 3);
}

TEST_F(ParserTests, Array_Init_Bool) {
  std::string input = "let arr [2]bool := [true, false];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto value = ASSERT_EXPR_ARRAY(node->Value.get(), 2);
  ASSERT_EXPR_BOOL(value->Elements[0].get(), true);
  ASSERT_EXPR_BOOL(value->Elements[1].get(), false);
}

TEST_F(ParserTests, Array_Init_String) {
  std::string input = "let arr [2]string := [\"test\", \"case\"];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto value = ASSERT_EXPR_ARRAY(node->Value.get(), 2);
  ASSERT_EXPR_STRING(value->Elements[0].get(), "test");
  ASSERT_EXPR_STRING(value->Elements[1].get(), "case");
}

// Verifies the parser's recursive ability to handle array-of-arrays
TEST_F(ParserTests, Array_Init_Nested) {
  std::string input = "let arr [2][2]int := [[1, 2], [3, 4]];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto value = ASSERT_EXPR_ARRAY(node->Value.get(), 2);
  ASSERT_EQ(value->Elements.size(), 2);

  auto first = ASSERT_EXPR_ARRAY(value->Elements[0].get(), 2);
  ASSERT_EXPR_NUMBER(first->Elements[0].get(), 1);
  ASSERT_EXPR_NUMBER(first->Elements[1].get(), 2);

  auto last = ASSERT_EXPR_ARRAY(value->Elements[1].get(), 2);
  ASSERT_EXPR_NUMBER(last->Elements[0].get(), 3);
  ASSERT_EXPR_NUMBER(last->Elements[1].get(), 4);
}

TEST_F(ParserTests, Array_Init_Float) {
  std::string input = "let arr [2]float := [1.5, 2.5];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto values = ASSERT_EXPR_ARRAY(node->Value.get(), 2);
  ASSERT_EXPR_FLOAT(values->Elements[0].get(), 1.5);
  ASSERT_EXPR_FLOAT(values->Elements[1].get(), 2.5);
}

TEST_F(ParserTests, Array_Access_Simple) {
  std::string input = "arr[0]";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto access = ASSERT_EXPR_ARRAY_ACCESS(stmts[0].get());
  ASSERT_EXPR_ID(access->Array.get(), "arr");
  ASSERT_EXPR_NUMBER(access->Index.get(), 0);
}

TEST_F(ParserTests, Array_Access_Chained) {
  std::string input = "matrix[1][2]";
  auto stmts = PARSE_PROGRAM(input, 1);

  auto access1 = ASSERT_EXPR_ARRAY_ACCESS(stmts[0].get());
  ASSERT_EXPR_NUMBER(access1->Index.get(), 2);

  auto access2 = ASSERT_EXPR_ARRAY_ACCESS(access1->Array.get());
  ASSERT_EXPR_NUMBER(access2->Index.get(), 1);
  ASSERT_EXPR_ID(access2->Array.get(), "matrix");
}

TEST_F(ParserTests, Array_Init_Empty) {
  std::string input = "let arr [0]int := [];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto values = ASSERT_EXPR_ARRAY(node->Value.get(), 0);
}

TEST_F(ParserTests, Array_Init_TrailingComma) {
  std::string input = "let arr [2]int := [1, 2,];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto values = ASSERT_EXPR_ARRAY(node->Value.get(), 2);

  ASSERT_EXPR_NUMBER(values->Elements[0].get(), 1);
  ASSERT_EXPR_NUMBER(values->Elements[1].get(), 2);
}

TEST_F(ParserTests, Array_Init_Expressions) {
  std::string input = "let arr [2]int := [1 + 2, x];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto values = ASSERT_EXPR_ARRAY(node->Value.get(), 2);

  auto add = ASSERT_EXPR_ADD(values->Elements[0].get());

  ASSERT_EXPR_NUMBER(add->Left.get(), 1);
  ASSERT_EXPR_NUMBER(add->Right.get(), 2);
  ASSERT_EXPR_ID(values->Elements[1].get(), "x");
}

TEST_F(ParserTests, Array_Init_Mixed_Types) {
  std::string input = "let arr [3]any := [1, \"text\", 3.14];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto arr = ASSERT_EXPR_ARRAY(node->Value.get(), 3);

  ASSERT_EXPR_NUMBER(arr->Elements[0].get(), 1);
  ASSERT_EXPR_STRING(arr->Elements[1].get(), "text");
  ASSERT_EXPR_FLOAT(arr->Elements[2].get(), 3.14);
}

TEST_F(ParserTests, Array_Init_Deeply_Nested_Error) {
  // Parser should handle this structure regardless of whether
  // the semantic analyzer eventually rejects it.
  std::string input = "let arr [2]int := [[[1]]];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto values = ASSERT_EXPR_ARRAY(node->Value.get(), 1);
  auto inner = ASSERT_EXPR_ARRAY(values->Elements[0].get(), 1);
  auto inner1 = ASSERT_EXPR_ARRAY(inner->Elements[0].get(), 1);

  ASSERT_EXPR_NUMBER(inner1->Elements[0].get(), 1);
}

TEST_F(ParserTests, Array_Init_Multiline) {
  std::string input = "let arr  [2]int := [\n 1,\n 2\n];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  ASSERT_EXPR_ARRAY(node->Value.get(), 2);
}

TEST_F(ParserTests, Array_Init_With_Expressions) {
  // Test that the parser handles binary operations inside the array
  std::string input = "let arr  [2]int := [1 + 2, 3 * 4, f(x)];";
  auto stmts = PARSE_PROGRAM(input, 1);
  auto node = ASSERT_EXPR_DECL(stmts[0].get(), "arr");
  auto arr = ASSERT_EXPR_ARRAY(node->Value.get(), 3);
}

TEST_F(ParserTests, Array_Init_Negative_Tests) {
  AssertParserError("let arr [3]int := [1, 2",
                    "Expected comma or ']' in array initializer", "", 23, 23);
}

TEST_F(ParserTests, Array_Init_Negative_Tests_3) {
  AssertParserError("let arr [3]int := [1, , 2];",
                    "Expected expression but got ,", "", 22, 23);
}

TEST_F(ParserTests, Array_Declaration_Negative_Tests) {
  AssertParserError("let arr [1 := [1];",
                    "expected close bracket in array or slice", "", 9, 10);
}

TEST_F(ParserTests, Array_Init_Unary_Negative) {
  AssertParserError("let arr [3]int := [+, 2];",
                    "Expected expression but got +", "", 19, 20);
}

TEST_F(ParserTests, Array_Access_Negative) {
  AssertParserError("let x [3]int := arr[1;", "Unexpected token: ```;```", "",
                    21, 22);
}

TEST_F(ParserTests, Array_Access_Negative_2) {
  AssertParserError("let x arr[1][2 := ;", "Unexpected token: ```[```", "", 9,
                    10);
}
