#include <gtest/gtest.h>
#include <iostream>
#include <lexer/keywords.h>
#include <memory>
#include <parser/parser.h>
#include <vector>

std::vector<Token> getTokens(const std::string &source) {
  Lexer lexer(source, KEYWORDS_ENGLISH);
  return lexer.tokenize();
}

void debugTokens(std::vector<Token> tokens) {
  for (auto tok : tokens) {
    std::cout << "TOKEN " << tok.value << "\n";
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
}

TEST(ParserTest, Conditional) {
  std::string input = "x = 0 ";
  auto tokens = getTokens(input);

  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto node = dynamic_cast<EqExpr *>(stmts[0].get());
  ASSERT_NE(node, nullptr);
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
}

TEST(ParserTest, ParseIf) {
  std::string input = "if x = 0 {print(\"x\");}";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto ifnode = dynamic_cast<IfExpr *>(stmts[0].get());
  ASSERT_NE(ifnode->Cond, nullptr);
  ASSERT_NE(ifnode->Success, nullptr);
  ASSERT_EQ(ifnode->Failure, nullptr);
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
  // FIXME add expects for the value and type
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

int main(int argc, char *argv[]) {
  testing::InitGoogleTest();
  int _ = RUN_ALL_TESTS();
  return 0;
}
