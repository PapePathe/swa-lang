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
