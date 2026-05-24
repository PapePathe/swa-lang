#include "lexer/lexer.h"
#include <cctype>
#include <cstddef>
#include <unordered_map>
#include <vector>

char Lexer::peek() { return pos < src.length() ? src[pos] : '\0'; }

char Lexer::get() {
  if (pos >= src.length()) {
    return '\0';
  }

  char c = src[pos++];

  if (c == '\n') {
    currentLine++;
    currentColumn = 1;
  } else {
    currentColumn++;
  }

  return c;
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (pos < src.length()) {
    int startLine = currentLine;
    int startCol = currentColumn;
    char c = peek();

    if (isspace(static_cast<unsigned char>(c))) {
      get();
      continue;
    }

    size_t startPosition = pos;

    if (c == '"') {
      get();
      std::string literal;

      while (peek() != '"' && pos < src.length()) {
        if (peek() == '\\') {
          get(); // Consume backslash
          char escaped = get();
          switch (escaped) {
          case 'n':
            literal += '\n';
            break;
          case 't':
            literal += '\t';
            break;
          case '\"':
            literal += '\"';
            break;
          case '\\':
            literal += '\\';
            break;
          default:
            literal += escaped;
            break;
          }
        } else {
          literal += get();
        }
      }

      if (pos >= src.length()) {
        auto tok = Token(TokenType::UNKNOWN, literal, {startPosition}, {pos});
        tokens.push_back(tok);
      } else {
        get(); // Consume closing quote
        auto tok = Token(TokenType::STRING, literal, {startPosition}, {pos});
        tokens.push_back(tok);
      }
      continue;
    }

    if (isdigit(static_cast<unsigned char>(c))) {
      size_t savedStart = pos;
      std::string num;
      bool hasDot = false;
      while (isdigit(peek()) || (peek() == '.' && !hasDot)) {
        if (peek() == '.')
          hasDot = true;
        num += get();
      }
      auto tok = Token({hasDot ? TokenType::NUMBER_FLOAT : TokenType::NUMBER,
                        num,
                        {savedStart},
                        {pos}});

      tokens.push_back(tok);
    } else if (isalpha(static_cast<unsigned char>(c)) || c == '_' ||
               static_cast<unsigned char>(c) > 127) {
      std::string ident;
      while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_' ||
             static_cast<unsigned char>(peek()) > 127) {
        ident += get();
      }

      if (keywords.count(ident)) {
        auto tok = Token({keywords.at(ident), ident, {startPosition}, {pos}});
        tokens.push_back(tok);
      } else {
        auto tok = Token(TokenType::IDENTIFIER, ident, {startPosition}, {pos});
        tokens.push_back(tok);
      }
    } else {
      char current = get();
      switch (current) {
      case '!': {
        if (peek() == '=') {
          get();
          auto tok = Token(TokenType::NOT_EQUALS, "!=", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok = Token(TokenType::NOT, "!", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }
      case '<': {
        if (peek() == '=') {
          get();
          auto tok =
              Token(TokenType::LESS_THAN_EQUALS, "<=", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok = Token(TokenType::LESS_THAN, "<", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }

      case '>': {
        if (peek() == '=') {
          get();
          auto tok = Token(TokenType::GREATER_THAN_EQUALS,
                           ">=", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok =
              Token(TokenType::GREATER_THAN, ">", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }

      case '+': {
        if (peek() == '=') {
          get();
          auto tok =
              Token(TokenType::PLUS_EQUALS, "+=", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok = Token(TokenType::PLUS, "+", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }

      case '-': {
        if (peek() == '=') {
          get();
          auto tok =
              Token(TokenType::MINUS_EQUALS, "-=", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok = Token(TokenType::MINUS, "-", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }

      case '/': {
        auto tok = Token(TokenType::DIVIDE, "/", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case '*': {
        if (peek() == '*') {
          get();
          auto tok =
              Token(TokenType::DOUBLE_STAR, "**", {startPosition}, {pos});
          tokens.push_back(tok);
        } else if (peek() == '=') {
          get();
          auto tok =
              Token(TokenType::STAR_EQUALS, "*=", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok = Token(TokenType::MULTIPLY, "*", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }
      case '=': {
        auto tok = Token(TokenType::EQUALS, "=", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case '&': {
        if (peek() == '&') {
          get();
          auto tok = Token(TokenType::AND, "&&", {startPosition}, {pos});
          tokens.push_back(tok);
        } else {
          auto tok = Token(TokenType::AMPERSAND, "&", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }
      case '|': {
        if (peek() == '|') {
          get();
          auto tok = Token(TokenType::OR, "||", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }
      case '.': {
        if (peek() == '.') {
          get(); // consume second .
          if (peek() == '.') {
            get();
            auto tok =
                Token(TokenType::VARIADIC, "...", {startPosition}, {pos});
            tokens.push_back(tok);
          }
        } else {
          auto tok = Token(TokenType::DOT, ".", {startPosition}, {pos});
          tokens.push_back(tok);
        }
        break;
      }
      case '(': {
        auto tok = Token(TokenType::OPEN_PAREN, "(", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case ')': {
        auto tok = Token(TokenType::CLOSE_PAREN, ")", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case '{': {
        auto tok = Token(TokenType::OPEN_CURLY, "{", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case '}': {
        auto tok = Token(TokenType::CLOSE_CURLY, "}", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }

      case '[': {
        auto tok = Token(TokenType::OPEN_BRACKET, "[", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case ']': {
        auto tok = Token(TokenType::CLOSE_BRACKET, "]", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }
      case ':': {
        if (peek() == '=') {
          get();
          auto tok = Token(TokenType::ASSIGNMENT, ":=", {startPosition}, {pos});
          tokens.push_back(tok);
          break;
        }

        auto tok = Token(TokenType::COLON, ":", {startPosition}, {pos});
        tokens.push_back(tok);
        break;
      }

      case ';':
        tokens.push_back(
            Token(TokenType::SEMICOLON, ";", {startPosition}, {pos}));
        break;
      case ',':
        tokens.push_back(Token(TokenType::COMMA, ",", {startPosition}, {pos}));
        break;
      default:
        tokens.push_back(Token(TokenType::UNKNOWN, std::string(1, current),
                               {startPosition}, {pos}));
        break;
      }
    }
  }

  auto tok = Token(TokenType::END_OF_FILE, "", {pos}, {pos});
  tokens.push_back(tok);
  return tokens;
}
