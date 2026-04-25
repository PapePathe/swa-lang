#include <gtest/gtest.h>
#include <lexer/keywords.hpp>
#include <parser/parser.h>

// Helper to get tokens from string for the parser
std::vector<Token> getTokens(const std::string &source) {
  Lexer lexer(source, KEYWORDS_ENGLISH);
  return lexer.tokenize();
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

TEST(ParserTest, ParseVariableDeclaration) {
  std::string input = "let x = 42;";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;

  ASSERT_EQ(stmts.size(), 1);

  auto declNode = dynamic_cast<DeclarationExpr *>(stmts[0].get());
  ASSERT_NE(declNode, nullptr);

  // Check if the variable name was captured correctly
  EXPECT_EQ(declNode->Name, "x");
}

TEST(ParserTest, ParseFunctionDefinition) {
  std::string input = "func my_func(a, b) { print(\"hi\"); }";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;

  auto funcNode = dynamic_cast<FuncExpr *>(stmts[0].get());
  ASSERT_NE(funcNode, nullptr);
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
  std::string input = "struct Point { x int, y float }";
  auto tokens = getTokens(input);
  Parser parser(tokens);

  auto program = parser.parseProgram();
  auto &stmts = program->Exprs;
  ASSERT_EQ(stmts.size(), 1);

  auto structNode = dynamic_cast<StructDefExpr *>(stmts[0].get());
  ASSERT_NE(structNode, nullptr);

  ASSERT_EQ(structNode->FieldTypes.size(), 2);
  ASSERT_NE(structNode->FieldTypes[0], nullptr);
  ASSERT_NE(structNode->FieldTypes[1], nullptr);

  ASSERT_EQ(structNode->FieldNames.size(), 2);
  ASSERT_EQ(structNode->FieldNames[0], "x");
  ASSERT_EQ(structNode->FieldNames[1], "y");
  ASSERT_EQ(structNode->Name, "Point");
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest();
  int _ = RUN_ALL_TESTS();
  return 0;
}
