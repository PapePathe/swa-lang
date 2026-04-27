#include "tokentype.h"
#include <assert.h>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Token {
  TokenType type;
  std::string value;
};

class Lexer {
  std::string src;
  std::unordered_map<std::string, TokenType> keywords;
  size_t pos = 0;

  char peek();
  char get();

public:
  Lexer(std::string s, std::unordered_map<std::string, TokenType> k)
      : src(s), pos(0), keywords(k) {}
  std::vector<Token> tokenize();
};
