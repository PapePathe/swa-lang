#ifndef SWA_LEXER
#define SWA_LEXER

#include "lexer/tokentype.h"
#include <assert.h>
#include <string>
#include <unordered_map>
#include <vector>

struct Token {
  TokenType type;
  std::string value;
  int line;
  int column;
};

class Lexer {
  std::string src;
  std::unordered_map<std::string, TokenType> keywords;
  size_t pos = 0;
  int currentLine;
  int currentColumn;

  char peek();
  char get();

public:
  Lexer(std::string s, std::unordered_map<std::string, TokenType> k)
      : src(s), pos(0), keywords(k) {}
  std::vector<Token> tokenize();
};
#endif // !SWA_LEXER
