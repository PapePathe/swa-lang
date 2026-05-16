#include <gtest/gtest.h>
#include <lexer/keywords.h>
#include <lexer/lexer.h>
#include <string>
#include <vector>

class LexerTest : public ::testing::Test {
protected:
  std::vector<Token>
  getTokens(const std::string &input,
            const std::unordered_map<std::string, TokenType> keywords) {
    Lexer lexer(input, keywords);
    return lexer.tokenize();
  }
};

std::string tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::DIALECT:
    return "DIALECT";
  case TokenType::FUNCTION:
    return "FUNCTION";
  case TokenType::CONST:
    return "CONST";
  case TokenType::OR:
    return "OR";
  case TokenType::LET:
    return "LET";
  case TokenType::NUMBER:
    return "NUMBER";
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::PLUS:
    return "PLUS";
  case TokenType::MINUS:
    return "MINUS";
  case TokenType::MULTIPLY:
    return "MULTIPLY";
  case TokenType::DIVIDE:
    return "DIVIDE";
  case TokenType::COLON:
    return "COLON";
  case TokenType::SEMICOLON:
    return "SEMICOLON";
  case TokenType::END_OF_FILE:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}

void AssertToken(const Token &t, TokenType expectedType,
                 const std::string &expectedValue) {
  EXPECT_EQ(t.type, expectedType)
      << tokenTypeToString(expectedType) << " do not match "
      << tokenTypeToString(t.type);
  EXPECT_EQ(t.value, expectedValue);
}

void AssertTokenSpan(const Token &token, TokenType expectedType,
                     const std::string &expectedLexeme, size_t expectedStart,
                     size_t expectedEnd) {
  EXPECT_EQ(token.type, expectedType);
  EXPECT_EQ(token.value, expectedLexeme);
  EXPECT_EQ(token.span.start.offset, expectedStart)
      << "Mismatched start offset for lexeme: " << expectedLexeme;
  EXPECT_EQ(token.span.end.offset, expectedEnd)
      << "Mismatched end offset for lexeme: " << expectedLexeme;
}

TEST_F(LexerTest, UnicodeIdentifiers) {
  auto tokens = getTokens("total_é π_var", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 3);
  AssertTokenSpan(tokens[0], TokenType::IDENTIFIER, "total_é", 0, 8);
  AssertTokenSpan(tokens[1], TokenType::IDENTIFIER, "π_var", 9, 15);
  AssertTokenSpan(tokens[2], TokenType::END_OF_FILE, "", 15, 15);
}

TEST_F(LexerTest, UnaryMinus) {
  auto tokens = getTokens("- 5", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 3);
  AssertTokenSpan(tokens[0], TokenType::MINUS, "-", 0, 1);
  AssertTokenSpan(tokens[1], TokenType::NUMBER, "5", 2, 3);
  AssertTokenSpan(tokens[2], TokenType::END_OF_FILE, "", 3, 3);
}

TEST_F(LexerTest, Not) {
  auto tokens = getTokens("!5", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 3);
  AssertTokenSpan(tokens[0], TokenType::NOT, "!", 0, 1);
  AssertTokenSpan(tokens[1], TokenType::NUMBER, "5", 1, 2);
  AssertTokenSpan(tokens[2], TokenType::END_OF_FILE, "", 2, 2);
}

TEST_F(LexerTest, KeywordsVsIdentifiers) {
  auto tokens =
      getTokens("let letter let123 return returning", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 6);
  AssertTokenSpan(tokens[0], TokenType::LET, "let", 0, 3);
  AssertTokenSpan(tokens[1], TokenType::IDENTIFIER, "letter", 4, 10);
  AssertTokenSpan(tokens[2], TokenType::IDENTIFIER, "let123", 11, 17);
  AssertTokenSpan(tokens[3], TokenType::RETURN, "return", 18, 24);
  AssertTokenSpan(tokens[4], TokenType::IDENTIFIER, "returning", 25, 34);
  AssertTokenSpan(tokens[5], TokenType::END_OF_FILE, "", 34, 34);
}

TEST_F(LexerTest, NumberLiterals) {
  auto tokens = getTokens("123 45.67 0.001", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 4);
  AssertTokenSpan(tokens[0], TokenType::NUMBER, "123", 0, 3);
  AssertTokenSpan(tokens[1], TokenType::FLOAT, "45.67", 4, 9);
  AssertTokenSpan(tokens[2], TokenType::FLOAT, "0.001", 10, 15);
  AssertTokenSpan(tokens[3], TokenType::END_OF_FILE, "", 15, 15);
}

TEST_F(LexerTest, HandlesBasicArithmetic) {
  auto tokens = getTokens("123 + 456", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 4);
  AssertTokenSpan(tokens[0], TokenType::NUMBER, "123", 0, 3);
  AssertTokenSpan(tokens[1], TokenType::PLUS, "+", 4, 5);
  AssertTokenSpan(tokens[2], TokenType::NUMBER, "456", 6, 9);
  AssertTokenSpan(tokens[3], TokenType::END_OF_FILE, "", 9, 9);
}

TEST_F(LexerTest, HandlesConditionalGt) {
  auto tokens = getTokens("x > 0", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 4);
  AssertTokenSpan(tokens[0], TokenType::IDENTIFIER, "x", 0, 1);
  AssertTokenSpan(tokens[1], TokenType::GREATER_THAN, ">", 2, 3);
  AssertTokenSpan(tokens[2], TokenType::NUMBER, "0", 4, 5);
  AssertTokenSpan(tokens[3], TokenType::END_OF_FILE, "", 5, 5);
}

TEST_F(LexerTest, HandlesAccentedIdentifiers) {
  auto tokens = getTokens("piñata + caffè * élite", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 6);
  AssertTokenSpan(tokens[0], TokenType::IDENTIFIER, "piñata", 0, 7);
  AssertTokenSpan(tokens[1], TokenType::PLUS, "+", 8, 9);
  AssertTokenSpan(tokens[2], TokenType::IDENTIFIER, "caffè", 10, 16);
  AssertTokenSpan(tokens[3], TokenType::MULTIPLY, "*", 17, 18);
  AssertTokenSpan(tokens[4], TokenType::IDENTIFIER, "élite", 19, 25);
  AssertTokenSpan(tokens[5], TokenType::END_OF_FILE, "", 25, 25);
}

TEST_F(LexerTest, HandlesComplexIdentifiers) {
  auto tokens = getTokens("_var123_tempé", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 2);
  AssertTokenSpan(tokens[0], TokenType::IDENTIFIER, "_var123_tempé", 0, 14);
  AssertTokenSpan(tokens[1], TokenType::END_OF_FILE, "", 14, 14);
}

TEST_F(LexerTest, IgnoresExtraneousWhitespace) {
  auto tokens = getTokens("  89   \n \t  +  \r  variable  ", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 4);
  AssertTokenSpan(tokens[0], TokenType::NUMBER, "89", 2, 4);
  AssertTokenSpan(tokens[1], TokenType::PLUS, "+", 12, 13);
  AssertTokenSpan(tokens[2], TokenType::IDENTIFIER, "variable", 18, 26);
  AssertTokenSpan(tokens[3], TokenType::END_OF_FILE, "", 28, 28);
}

TEST_F(LexerTest, HandlesCompoundOperators) {
  auto tokens = getTokens("x += 5 = y && z || a *= 2", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 12);
  AssertTokenSpan(tokens[0], TokenType::IDENTIFIER, "x", 0, 1);
  AssertTokenSpan(tokens[1], TokenType::PLUS_EQUALS, "+=", 2, 4);
  AssertTokenSpan(tokens[2], TokenType::NUMBER, "5", 5, 6);
  AssertTokenSpan(tokens[3], TokenType::EQUALS, "=", 7, 8);
  AssertTokenSpan(tokens[4], TokenType::IDENTIFIER, "y", 9, 10);
  AssertTokenSpan(tokens[5], TokenType::AND, "&&", 11, 13);
  AssertTokenSpan(tokens[6], TokenType::IDENTIFIER, "z", 14, 15);
  AssertTokenSpan(tokens[7], TokenType::OR, "||", 16, 18);
  AssertTokenSpan(tokens[8], TokenType::IDENTIFIER, "a", 19, 20);
  AssertTokenSpan(tokens[9], TokenType::STAR_EQUALS, "*=", 21, 23);
  AssertTokenSpan(tokens[10], TokenType::NUMBER, "2", 24, 25);
  AssertTokenSpan(tokens[11], TokenType::END_OF_FILE, "", 25, 25);
}

TEST_F(LexerTest, DistinguishesSingleFromDouble) {
  auto tokens = getTokens("+ += := =", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 5);
  AssertTokenSpan(tokens[0], TokenType::PLUS, "+", 0, 1);
  AssertTokenSpan(tokens[1], TokenType::PLUS_EQUALS, "+=", 2, 4);
  AssertTokenSpan(tokens[2], TokenType::ASSIGNMENT, ":=", 5, 7);
  AssertTokenSpan(tokens[3], TokenType::EQUALS, "=", 8, 9);
  AssertTokenSpan(tokens[4], TokenType::END_OF_FILE, "", 9, 9);
}

TEST_F(LexerTest, EmptyString) {
  auto tokens = getTokens("", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 1);
  AssertTokenSpan(tokens[0], TokenType::END_OF_FILE, "", 0, 0);
}

TEST_F(LexerTest, LetStatement) {
  auto tokens = getTokens("let x := 42;", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 6);
  AssertTokenSpan(tokens[0], TokenType::LET, "let", 0, 3);
  AssertTokenSpan(tokens[1], TokenType::IDENTIFIER, "x", 4, 5);
  AssertTokenSpan(tokens[2], TokenType::ASSIGNMENT, ":=", 6, 8);
  AssertTokenSpan(tokens[3], TokenType::NUMBER, "42", 9, 11);
  AssertTokenSpan(tokens[4], TokenType::SEMICOLON, ";", 11, 12);
  AssertTokenSpan(tokens[5], TokenType::END_OF_FILE, "", 12, 12);
}

TEST_F(LexerTest, StringExpression) {
  auto tokens = getTokens("\"Salam\"", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 2);
  AssertTokenSpan(tokens[0], TokenType::STRING, "Salam", 0, 7);
  AssertTokenSpan(tokens[1], TokenType::END_OF_FILE, "", 7, 7);
}

TEST_F(LexerTest, StructDeclaration) {
  auto tokens = getTokens("struct Point { x int }", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 7);
  AssertTokenSpan(tokens[0], TokenType::STRUCT, "struct", 0, 6);
  AssertTokenSpan(tokens[1], TokenType::IDENTIFIER, "Point", 7, 12);
  AssertTokenSpan(tokens[2], TokenType::OPEN_CURLY, "{", 13, 14);
  AssertTokenSpan(tokens[3], TokenType::IDENTIFIER, "x", 15, 16);
  AssertTokenSpan(tokens[4], TokenType::INT, "int", 17, 20);
  AssertTokenSpan(tokens[5], TokenType::CLOSE_CURLY, "}", 21, 22);
  AssertTokenSpan(tokens[6], TokenType::END_OF_FILE, "", 22, 22);
}

TEST_F(LexerTest, StructDeclarationFloat) {
  auto tokens = getTokens("struct Point { x float }", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 7);
  AssertTokenSpan(tokens[0], TokenType::STRUCT, "struct", 0, 6);
  AssertTokenSpan(tokens[1], TokenType::IDENTIFIER, "Point", 7, 12);
  AssertTokenSpan(tokens[2], TokenType::OPEN_CURLY, "{", 13, 14);
  AssertTokenSpan(tokens[3], TokenType::IDENTIFIER, "x", 15, 16);
  AssertTokenSpan(tokens[4], TokenType::FLOAT, "float", 17, 22);
  AssertTokenSpan(tokens[5], TokenType::CLOSE_CURLY, "}", 23, 24);
  AssertTokenSpan(tokens[6], TokenType::END_OF_FILE, "", 24, 24);
}

TEST_F(LexerTest, StringExpressionWithNumbers) {
  auto tokens = getTokens("\"Age: 123\"", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 2);
  AssertTokenSpan(tokens[0], TokenType::STRING, "Age: 123", 0, 10);
  AssertTokenSpan(tokens[1], TokenType::END_OF_FILE, "", 10, 10);
}

TEST_F(LexerTest, PrintAndPrintF) {
  auto tokens = getTokens("print print_f", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 3);
  AssertTokenSpan(tokens[0], TokenType::PRINT, "print", 0, 5);
  AssertTokenSpan(tokens[1], TokenType::PRINT_F, "print_f", 6, 13);
  AssertTokenSpan(tokens[2], TokenType::END_OF_FILE, "", 13, 13);
}

TEST_F(LexerTest, BlankString) {
  auto tokens = getTokens(" ", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 1);
  AssertTokenSpan(tokens[0], TokenType::END_OF_FILE, "", 1, 1);
}

TEST_F(LexerTest, Dialect) {
  auto tokens = getTokens("dialect:english;", KEYWORDS_ENGLISH);

  ASSERT_EQ(tokens.size(), 5);
  AssertTokenSpan(tokens[0], TokenType::DIALECT, "dialect", 0, 7);
  AssertTokenSpan(tokens[1], TokenType::COLON, ":", 7, 8);
  AssertTokenSpan(tokens[2], TokenType::IDENTIFIER, "english", 8, 15);
  AssertTokenSpan(tokens[3], TokenType::SEMICOLON, ";", 15, 16);
  AssertTokenSpan(tokens[4], TokenType::END_OF_FILE, "", 16, 16);
}

TEST_F(LexerTest, AllDialectKeywordsAreEqual) {
  ASSERT_EQ(KEYWORDS_ENGLISH.size(), 35);
  ASSERT_EQ(KEYWORDS_FRENCH.size(), 24);
}

TEST_F(LexerTest, FrencDialect) {
  auto tokens =
      getTokens("dialecte constante fonction sinon si tantque demarrer "
                "afficher retourner structure variable entier entier64 octet "
                "erreur decimal chaine booleen et ou vrai faux variadique zero",
                KEYWORDS_FRENCH);

  ASSERT_EQ(tokens.size(), 25);
  AssertTokenSpan(tokens[0], TokenType::DIALECT, "dialecte", 0, 8);
  AssertTokenSpan(tokens[1], TokenType::CONST, "constante", 9, 18);
  AssertTokenSpan(tokens[3], TokenType::ELSE, "sinon", 28, 33);
  AssertTokenSpan(tokens[4], TokenType::IF, "si", 34, 36);
  AssertTokenSpan(tokens[5], TokenType::WHILE, "tantque", 37, 44);
  AssertTokenSpan(tokens[6], TokenType::MAIN, "demarrer", 45, 53);
  AssertTokenSpan(tokens[7], TokenType::PRINT, "afficher", 54, 62);
  AssertTokenSpan(tokens[8], TokenType::RETURN, "retourner", 63, 72);
  AssertTokenSpan(tokens[9], TokenType::STRUCT, "structure", 73, 82);
  AssertTokenSpan(tokens[10], TokenType::LET, "variable", 83, 91);
  AssertTokenSpan(tokens[11], TokenType::INT, "entier", 92, 98);
  AssertTokenSpan(tokens[12], TokenType::INT, "entier64", 99, 107);
  AssertTokenSpan(tokens[13], TokenType::BYTE, "octet", 108, 113);
  AssertTokenSpan(tokens[14], TokenType::ERROR, "erreur", 114, 120);
  AssertTokenSpan(tokens[15], TokenType::FLOAT, "decimal", 121, 128);
  AssertTokenSpan(tokens[16], TokenType::STRING, "chaine", 129, 135);
  AssertTokenSpan(tokens[17], TokenType::BOOL, "booleen", 136, 143);
  AssertTokenSpan(tokens[18], TokenType::AND, "et", 144, 146);
  AssertTokenSpan(tokens[19], TokenType::OR, "ou", 147, 149);
  AssertTokenSpan(tokens[20], TokenType::TRUE, "vrai", 150, 154);
  AssertTokenSpan(tokens[21], TokenType::FALSE, "faux", 155, 159);
  AssertTokenSpan(tokens[22], TokenType::VARIADIC, "variadique", 160, 170);
  AssertTokenSpan(tokens[23], TokenType::ZERO, "zero", 171, 175);
  AssertTokenSpan(tokens[24], TokenType::END_OF_FILE, "", 175, 175);
}
