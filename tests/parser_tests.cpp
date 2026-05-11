#include "ast/node.h"
#include <exception>
#include <gtest/gtest.h>
#include <iostream>
#include <lexer/keywords.h>
#include <lexer/tokentypestring.h>
#include <memory>
#include <parser/parser.h>
#include <stdexcept>
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

TEST(ParserTest, ParseMinimalProgram) {
  std::string input = "main()int { print(\"Salam\"); }";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 0);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 0);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
}

TEST(ParserTest, ParseMinimalProgramWithCommandArgs) {
  std::string input =
      "main(arguments_count int, arguments []string)int { return 0; }";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 2);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 2);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
}

TEST(ParserTest, ParseMinimalProgramWithCommandArgsAndEnv) {
  std::string input = "main(arguments_count int, arguments []string, "
                      "environment []string)int { return 0; }";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

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

TEST(ParserTest, ParseMinimalProgramWithDialect) {
  std::string input = R"(
    dialect:english;
    main() int {
      print("Salam");
    }
  )";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->Proto->Name, "main");
  ASSERT_EQ(node->Proto->Args.size(), 0);
  ASSERT_EQ(node->Proto->ArgsTypes.size(), 0);
  ASSERT_EQ(node->Body.get()->Exprs.size(), 1);
}

TEST(ParserTest, ParseEmptyPrintStatement) {
  std::string input = "print();";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST(ParserTest, ParseEmptyPrintFStatement) {
  std::string input = "print_f();";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST(ParserTest, ParseInvalidPrintFStatement) {
  std::string input = "print_f(x);";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  ASSERT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST(ParserTest, ParsePrintFStatement) {
  std::string input = "print_f(\"Hello\");";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto printNode = dynamic_cast<Formatted_Print_Expr *>(stmts[0].get());
  ASSERT_NE(printNode, nullptr);
  ASSERT_EQ(printNode->Values.size(), 1);

  auto str = dynamic_cast<StrExpr *>(printNode->Values[0].get());
  ASSERT_NE(str, nullptr);
  ASSERT_EQ(str->Name, "Hello");
}

TEST(ParserTest, ParsePrintStatement) {
  std::string input = "print(\"Hello\");";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto printNode = dynamic_cast<PrintExpr *>(stmts[0].get());
  ASSERT_NE(printNode, nullptr);
  ASSERT_EQ(printNode->Values.size(), 1);

  auto str = dynamic_cast<StrExpr *>(printNode->Values[0].get());
  ASSERT_NE(str, nullptr);
  ASSERT_EQ(str->Name, "Hello");
}

TEST(ParserTest, ParsePrintStatement2) {
  std::string input = R"(
      print("10 - 3 - 2 = %d", a);
  )";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto printNode = dynamic_cast<PrintExpr *>(stmts[0].get());
  ASSERT_NE(printNode, nullptr);
  ASSERT_EQ(printNode->Values.size(), 2);

  auto str = dynamic_cast<StrExpr *>(printNode->Values[0].get());
  ASSERT_NE(str, nullptr);
  ASSERT_EQ(str->Name, "10 - 3 - 2 = %d");

  auto id = dynamic_cast<IdExpr *>(printNode->Values[1].get());
  ASSERT_NE(id, nullptr);
  ASSERT_EQ(id->Name, "a");
}

TEST(ParserTest, ParsePrintStatement3) {
  std::string input = R"(
  -5
  )";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  //  debugTokens(tokens);
  auto program = parser.parseProgram();

  ASSERT_NE(program, nullptr);

  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<UnaryMinusExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto val = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_NE(val, nullptr);
  ASSERT_EQ(val->Value, 5);
}

TEST(ParserTest, AddExpr) {
  std::string input = "x + 0 ";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<AddExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, CompositeBinaryExpr1) {
  std::string input = "2 + 3 * 4";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<AddExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<NumberExpr *>(node->Left.get());
  ASSERT_EQ(left->Value, 2);

  auto mul = dynamic_cast<MulExpr *>(node->Right.get());
  ASSERT_NE(mul, nullptr);

  auto mulleft = dynamic_cast<NumberExpr *>(mul->Left.get());
  ASSERT_EQ(mulleft->Value, 3);

  auto mulright = dynamic_cast<NumberExpr *>(mul->Right.get());
  ASSERT_EQ(mulright->Value, 4);
}

TEST(ParserTest, CompositeBinaryExpr2) {
  std::string input = "(2 + 3) * 4";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<MulExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto add = dynamic_cast<AddExpr *>(node->Left.get());
  ASSERT_NE(add, nullptr);

  auto left = dynamic_cast<NumberExpr *>(add->Left.get());
  ASSERT_EQ(left->Value, 2);

  auto right = dynamic_cast<NumberExpr *>(add->Right.get());
  ASSERT_EQ(right->Value, 3);

  auto num = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_NE(num, nullptr);
  ASSERT_EQ(num->Value, 4);
}

TEST(ParserTest, SubExpr) {
  std::string input = "x - 0 ";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<SubExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, MulExpr) {
  std::string input = "x * 0 ";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<MulExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, DivExpr) {
  std::string input = "x / 0 ";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<DivExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
}

// FIXME
// TEST(ParserTest, ConditionalNotEq) {
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

TEST(ParserTest, ConditionalEq) {
  std::string input = "x = 0 ";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<EqExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ConditionalGt) {
  std::string input = "x > 0 ";
  auto tokens = getTokens(input);
  //  debugTokens(tokens);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<GTExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ConditionalGte) {
  std::string input = "x >= 0 ";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<GTEExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ConditionalLt) {
  std::string input = "x < 0 ";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<LTExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ConditionalLte) {
  std::string input = "x <= 0 ";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<LTEExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);

  auto left = dynamic_cast<IdExpr *>(node->Left.get());
  ASSERT_EQ(left->Name, "x");

  auto right = dynamic_cast<NumberExpr *>(node->Right.get());
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ParseIf) {
  std::string input = "if x = 0 {print(\"x\");}";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto ifnode = dynamic_cast<IfExpr *>(stmts[0].get());
  ASSERT_NE(ifnode->Success, nullptr);
  ASSERT_EQ(ifnode->Failure, nullptr);
  ASSERT_NE(ifnode->Cond, nullptr);

  auto cond = dynamic_cast<EqExpr *>(ifnode->Cond.get());
  auto left = dynamic_cast<IdExpr *>(cond->Left.get());
  auto right = dynamic_cast<NumberExpr *>(cond->Right.get());

  ASSERT_EQ(left->Name, "x");
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ParseElse) {
  std::string input = "if x = 0 { print(\"x\"); }"
                      "else { print(\"x\"); }";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto ifnode = dynamic_cast<IfExpr *>(stmts[0].get());
  ASSERT_NE(ifnode->Cond, nullptr);
  ASSERT_NE(ifnode->Success, nullptr);
  ASSERT_NE(ifnode->Failure, nullptr);

  auto cond = dynamic_cast<EqExpr *>(ifnode->Cond.get());
  auto left = dynamic_cast<IdExpr *>(cond->Left.get());
  auto right = dynamic_cast<NumberExpr *>(cond->Right.get());

  ASSERT_EQ(left->Name, "x");
  ASSERT_EQ(right->Value, 0);
}

TEST(ParserTest, ParseVariableDeclaration) {
  std::string input = "let x int := 42;";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto declNode = dynamic_cast<DeclarationExpr *>(stmts[0].get());
  ASSERT_NE(declNode, nullptr);
  EXPECT_EQ(declNode->Name, "x");

  auto val = dynamic_cast<NumberExpr *>(declNode->Value.get());
  EXPECT_EQ(val->Value, 42);

  // FIXME assert declNode->T == TypeInt
}

TEST(ParserTest, ParseFunctionDefinition) {
  std::string input =
      "func my_func(a int, b float, c bool, d string, e byte) int "
      "{ print(\"hi\"); }";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
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

TEST(ParserTest, ParseStructDefinition) {
  std::string input = "struct Point { x int }";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto structNode = dynamic_cast<StructDefExpr *>(stmts[0].get());
  ASSERT_NE(structNode, nullptr);
  ASSERT_EQ(structNode->Name, "Point");
  ASSERT_EQ(structNode->FieldNames.size(), 1);
  ASSERT_EQ(structNode->FieldNames[0], "x");
}

TEST(ParserTest, ParseStructDefinitionAllTypes) {
  std::string input =
      "struct Point { x int, y float, a string, b bool, c byte, "
      "d [2]int}";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

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

TEST(ParserTest, Return) {
  std::string input = "return 0;";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto returnexpr = dynamic_cast<ReturnExpr *>(stmts[0].get());
  ASSERT_NE(returnexpr, nullptr);

  auto v = dynamic_cast<NumberExpr *>(returnexpr->Value.get());
  ASSERT_NE(v, nullptr);
  ASSERT_EQ(v->Value, 0);
}

// TEST(ParserTest, MissingClosingParen) {
//   std::string input = "myFunc(1, 2";
//   auto tokens = getTokens(input);
//   Parser parser(tokens);
//
//   EXPECT_THROW(parser.parseProgram(), std::runtime_error);
// }

TEST(ParserTest, TrailingComma) {
  std::string input = "myFunc(1, )";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto call = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 1);

  auto arg = dynamic_cast<NumberExpr *>(call->Args[0].get());
  EXPECT_NE(arg, nullptr);
  EXPECT_EQ(arg->Value, 1);
}

TEST(ParserTest, EmptyArgInMiddle) {
  std::string input = "myFunc(1, , 3)";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  EXPECT_THROW(parser.parseProgram(), std::runtime_error);
}

TEST(ParserTest, MultipleArguments) {
  std::string input = "add(x, 10, \"result\")";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto call = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 3);

  EXPECT_TRUE(dynamic_cast<IdExpr *>(call->Args[0].get()));
  EXPECT_TRUE(dynamic_cast<NumberExpr *>(call->Args[1].get()));
  EXPECT_TRUE(dynamic_cast<StrExpr *>(call->Args[2].get()));
}

TEST(ParserTest, NestedFunctionCalls) {
  std::string input = "highOrderFunc(square(abs(y)))";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto outer = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(outer, nullptr);
  ASSERT_EQ(outer->Args.size(), 1);

  auto outerCallee = dynamic_cast<IdExpr *>(outer->Callee.get());
  ASSERT_NE(outerCallee, nullptr);
  ASSERT_EQ(outerCallee->Name, "highOrderFunc");

  auto middle = dynamic_cast<CallExpr *>(outer->Args[0].get());
  ASSERT_NE(middle, nullptr);
  ASSERT_EQ(middle->Args.size(), 1);

  auto middleCallee = dynamic_cast<IdExpr *>(middle->Callee.get());
  ASSERT_NE(middleCallee, nullptr);
  ASSERT_EQ(middleCallee->Name, "square");

  auto inner = dynamic_cast<CallExpr *>(middle->Args[0].get());
  ASSERT_NE(inner, nullptr);
  ASSERT_EQ(inner->Args.size(), 1);

  auto innerCallee = dynamic_cast<IdExpr *>(inner->Callee.get());
  ASSERT_NE(innerCallee, nullptr);
  ASSERT_EQ(innerCallee->Name, "abs");
}

TEST(ParserTest, FunctionCallInArithmetic) {
  std::string input = "5 + getVal() * 2";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto add = dynamic_cast<AddExpr *>(program->Exprs[0].get());
  ASSERT_NE(add, nullptr);

  auto mul = dynamic_cast<MulExpr *>(add->Right.get());
  ASSERT_NE(mul, nullptr);

  auto call = dynamic_cast<CallExpr *>(mul->Left.get());
  ASSERT_NE(call, nullptr);

  auto callee = dynamic_cast<IdExpr *>(call->Callee.get());
  EXPECT_EQ(callee->Name, "getVal");
}

TEST(ParserTest, SimpleFunctionCallNoArgs) {
  std::string input = "myFunc()";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto call = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(call, nullptr);

  auto callee = dynamic_cast<IdExpr *>(call->Callee.get());
  EXPECT_EQ(callee->Name, "myFunc");
  EXPECT_EQ(call->Args.size(), 0);
}

TEST(ParserTest, FunctionCallWithSingleArg) {
  std::string input = "myFunc(42)";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto call = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->Args.size(), 1);

  auto callee = dynamic_cast<IdExpr *>(call->Callee.get());
  EXPECT_EQ(callee->Name, "myFunc");

  auto arg = dynamic_cast<NumberExpr *>(call->Args[0].get());
  ASSERT_NE(arg, nullptr);
  ASSERT_EQ(arg->Value, 42);
}

TEST(ParserTest, NestedFunctionCall) {
  std::string input = "exponent(calculate(1 + 2))";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto call = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 1);

  auto callee = dynamic_cast<IdExpr *>(call->Callee.get());
  EXPECT_EQ(callee->Name, "exponent");

  auto inner = dynamic_cast<CallExpr *>(call->Args[0].get());
  ASSERT_NE(inner, nullptr);
  ASSERT_EQ(inner->Args.size(), 1);

  auto innerCallee = dynamic_cast<IdExpr *>(inner->Callee.get());
  ASSERT_NE(innerCallee, nullptr);
  EXPECT_EQ(innerCallee->Name, "calculate");
}

TEST(ParserTest, FunctionCallPrecedenceInLogic) {
  std::string input = "isTrue() && check(5 + 2)";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto and_expr = dynamic_cast<Logical_And_Expr *>(program->Exprs[0].get());
  ASSERT_NE(and_expr, nullptr);

  auto left = dynamic_cast<CallExpr *>(and_expr->Left.get());
  ASSERT_NE(left, nullptr);

  auto lcallee = dynamic_cast<IdExpr *>(left->Callee.get());
  ASSERT_EQ(lcallee->Name, "isTrue");
  ASSERT_EQ(left->Args.size(), 0);

  auto right = dynamic_cast<CallExpr *>(and_expr->Right.get());
  ASSERT_NE(right, nullptr);
  ASSERT_EQ(right->Args.size(), 1);

  auto rcallee = dynamic_cast<IdExpr *>(right->Callee.get());
  ASSERT_EQ(rcallee->Name, "check");
}

TEST(ParserTest, FunctionCallPrecedenceInLogic2) {
  std::string input = "isTrue() || check(5 + 2)";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto and_expr = dynamic_cast<Logical_Or_Expr *>(program->Exprs[0].get());
  ASSERT_NE(and_expr, nullptr);

  auto left = dynamic_cast<CallExpr *>(and_expr->Left.get());
  ASSERT_NE(left, nullptr);

  auto lcallee = dynamic_cast<IdExpr *>(left->Callee.get());
  ASSERT_NE(lcallee, nullptr);
  ASSERT_EQ(lcallee->Name, "isTrue");
  ASSERT_EQ(left->Args.size(), 0);

  auto right = dynamic_cast<CallExpr *>(and_expr->Right.get());
  ASSERT_NE(right, nullptr);
  ASSERT_EQ(right->Args.size(), 1);

  auto rcallee = dynamic_cast<IdExpr *>(right->Callee.get());
  ASSERT_EQ(rcallee->Name, "check");
}

TEST(ParserTest, CallWithinAssignment) {
  std::string input = "let x int := getVal() + 5;";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto decl = dynamic_cast<DeclarationExpr *>(program->Exprs[0].get());
  ASSERT_NE(decl, nullptr);
}

TEST(ParserTest, CallsAsArgumentsInMath) {
  std::string input = "calculate(sum(1, 2) * 3, is_valid(x) && true)";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto call = dynamic_cast<CallExpr *>(program->Exprs[0].get());
  ASSERT_NE(call, nullptr);
  ASSERT_EQ(call->Args.size(), 2);

  // First Arg: sum(1,2) * 3 (MulExpr)
  auto firstArg = dynamic_cast<MulExpr *>(call->Args[0].get());
  ASSERT_NE(firstArg, nullptr);

  auto mleft = dynamic_cast<CallExpr *>(firstArg->Left.get());
  ASSERT_NE(mleft, nullptr);

  auto mright = dynamic_cast<NumberExpr *>(firstArg->Right.get());
  ASSERT_NE(mright, nullptr);

  auto secondArg = dynamic_cast<Logical_And_Expr *>(call->Args[1].get());
  ASSERT_NE(secondArg, nullptr);

  auto lleft = dynamic_cast<CallExpr *>(secondArg->Left.get());
  ASSERT_NE(lleft, nullptr);

  auto lright = dynamic_cast<BoolExpr *>(secondArg->Right.get());
  ASSERT_NE(lright, nullptr);
  ASSERT_EQ(lright->Value, true);
}

TEST(ParserTest, CallInArrayInitialization) {
  std::string input = "let buffer []byte := generate_buffer(size());";
  auto tokens = getTokens(input);
  Parser parser(tokens);
  auto program = parser.parseProgram();

  auto decl = dynamic_cast<DeclarationExpr *>(program->Exprs[0].get());
  ASSERT_NE(decl, nullptr);

  auto call = dynamic_cast<CallExpr *>(decl->Value.get());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->Args.size(), 1);
}

TEST(ParserTest, CallInsideExpressionSemicolonLoophole) {
  std::string input = "let x int := getVal() + 1;";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();

  auto decl = dynamic_cast<DeclarationExpr *>(program->Exprs[0].get());
  ASSERT_NE(decl, nullptr);

  auto add = dynamic_cast<AddExpr *>(decl->Value.get());
  ASSERT_NE(add, nullptr);

  auto call = dynamic_cast<CallExpr *>(add->Left.get());
  ASSERT_NE(call, nullptr);

  auto num = dynamic_cast<NumberExpr *>(add->Right.get());
  ASSERT_NE(num, nullptr);
  ASSERT_EQ(num->Value, 1);
}
