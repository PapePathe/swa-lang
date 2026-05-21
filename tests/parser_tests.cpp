#include "ast/node.h"
#include "lexer/lexer.h"
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
    std::cout << "TOKEN " << tok.value << " " << tokenTypeString(tok.type)
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

  std::vector<std::unique_ptr<Expr>> PARSE_PROGRAM(const std::string &input,
                                                   size_t expectedCount) {
    auto tokens = getTokens(input);
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

TEST(SemanticTest, ThrowsErrorOnInvalidMainSignature) {
  // Invalid: arguments should be []string, not int
  std::string input = "main(argc int, argv int) int { return 0; }";

  auto tokens = getTokens(input);
  Parser parser(tokens);

  try {
    parser.parseProgram();
    FAIL() << "Expected std::runtime_error was not thrown";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ(e.what(),
                 "second argument of main should be a slice of strings");
  } catch (std::exception e) {
    FAIL() << "Expected std::runtime_error, but caught a different type ("
           << e.what() << ")";
  }
}

TEST(SemanticTest, ThrowsErrorOnInvalidMainSignature2) {
  std::string input = "main(argc string) int { return 0; }";

  auto tokens = getTokens(input);
  Parser parser(tokens);

  try {
    parser.parseProgram();
    FAIL() << "Expected std::runtime_error was not thrown";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ(e.what(), "first argument of main should be of TypeInt");
  } catch (std::exception e) {
    FAIL() << "Expected std::runtime_error, but caught a different type ("
           << e.what() << ")";
  }
}

TEST(SemanticTest, ThrowsErrorOnInvalidMainSignature3) {
  std::string input = "main(a int, b []string, c string) int { return 0; }";

  auto tokens = getTokens(input);
  Parser parser(tokens);

  try {
    parser.parseProgram();
    FAIL() << "Expected std::runtime_error was not thrown";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ(e.what(),
                 "third argument of main should be a slice of strings");
  } catch (std::exception e) {
    FAIL() << "Expected std::runtime_error, but caught a different type ("
           << e.what() << ")";
  }
}

TEST(SemanticTest, ThrowsErrorOnInvalidMainSignature4) {
  std::string input =
      "main(a int, b []string, c []string, d int) int { return 0; }";

  auto tokens = getTokens(input);
  Parser parser(tokens);

  try {
    parser.parseProgram();
    FAIL() << "Expected std::runtime_error was not thrown";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ(e.what(), "main should have at most 3 arguments");
  } catch (std::exception e) {
    FAIL() << "Expected std::runtime_error, but caught a different type ("
           << e.what() << ")";
  }
}

TEST(SemanticTest, ThrowsErrorOnInvalidMainSignature5) {
  std::string input = "main() string { return 0; }";

  auto tokens = getTokens(input);
  Parser parser(tokens);

  try {
    parser.parseProgram();
    FAIL() << "Expected std::runtime_error was not thrown";
  } catch (const std::runtime_error &e) {
    EXPECT_STREQ(e.what(), "return value of main should be TypeInt");
  } catch (std::exception e) {
    FAIL() << "Expected std::runtime_error, but caught a different type ("
           << e.what() << ")";
  }
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

  ASSERT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST_F(ParserTests, ParseEmptyPrintFStatement) {
  std::string input = "print_f();";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST_F(ParserTests, ParseInvalidPrintFStatement) {
  std::string input = "print_f(x);";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), std::runtime_error);
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

TEST_F(ParserTests, EmptyArgInMiddle) {
  std::string input = "myFunc(1, , 3)";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  EXPECT_THROW(parser.parseProgram(), std::runtime_error);
}

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
