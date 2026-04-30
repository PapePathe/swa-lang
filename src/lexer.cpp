#include "lexer/lexer.h"
#include <cctype>
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
        // You might want a specific error for unterminated strings
        tokens.push_back({TokenType::UNKNOWN, literal});
      } else {
        get(); // Consume closing quote
        tokens.push_back({TokenType::STRING, literal});
      }
      continue;
    }

    if (isdigit(static_cast<unsigned char>(c))) {
      std::string num;
      bool hasDot = false;
      while (isdigit(peek()) || (peek() == '.' && !hasDot)) {
        if (peek() == '.')
          hasDot = true;
        num += get();
      }
      tokens.push_back({hasDot ? TokenType::FLOAT : TokenType::NUMBER, num});
    }

    else if (isalpha(static_cast<unsigned char>(c)) || c == '_' ||
             static_cast<unsigned char>(c) > 127) {
      std::string ident;
      while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_' ||
             static_cast<unsigned char>(peek()) > 127) {
        ident += get();
      }

      if (keywords.count(ident)) {
        tokens.push_back({keywords.at(ident), ident});
      } else {
        tokens.push_back({TokenType::IDENTIFIER, ident});
      }
    }

    else {
      char current = get();
      switch (current) {
      case '!':
        if (peek() == '=') {
          get();
          tokens.push_back({TokenType::NOT_EQUALS, "!="});
        } else {
          tokens.push_back({TokenType::NOT, "!"});
        }
        break;
      case '<':
        if (peek() == '=') {
          get();
          tokens.push_back({TokenType::LESS_THAN_EQUALS, "<="});
        } else {
          tokens.push_back({TokenType::LESS_THAN, "<"});
        }
        break;
      case '>':
        if (peek() == '=') {
          get();
          tokens.push_back({TokenType::GREATER_THAN_EQUALS, ">="});
        } else {
          tokens.push_back({TokenType::GREATER_THAN, ">"});
        }
        break;
      case '+':
        if (peek() == '=') {
          get();
          tokens.push_back({TokenType::PLUS_EQUALS, "+="});
        } else {
          tokens.push_back({TokenType::PLUS, "+"});
        }
        break;
      case '-':
        if (peek() == '=') {
          get();
          tokens.push_back({TokenType::MINUS_EQUALS, "-="});
        } else {
          tokens.push_back({TokenType::MINUS, "-"});
        }
        break;
      case '/':
        tokens.push_back({TokenType::DIVIDE, "/"});
        break;
      case '*':
        if (peek() == '*') {
          get();
          tokens.push_back({TokenType::DOUBLE_STAR, "**"});
        } else if (peek() == '=') {
          get();
          tokens.push_back({TokenType::STAR_EQUALS, "*="});
        } else {
          tokens.push_back({TokenType::MULTIPLY, "*"});
        }
        break;
      case '=':
        tokens.push_back({TokenType::EQUALS, "="});
        break;
      case '&':
        if (peek() == '&') {
          get();
          tokens.push_back({TokenType::AND, "&&"});
        } else {
          tokens.push_back({TokenType::AMPERSAND, "&"});
        }
        break;
      case '|':
        if (peek() == '|') {
          get();
          tokens.push_back({TokenType::OR, "||"});
        }
        break;
      case '.':
        if (peek() == '.') {
          get(); // consume second .
          if (peek() == '.') {
            get();
            tokens.push_back({TokenType::VARIADIC, "..."});
          }
        } else {
          tokens.push_back({TokenType::DOT, "."});
        }
        break;
      case '(':
        tokens.push_back({TokenType::OPEN_PAREN, "("});
        break;
      case ')':
        tokens.push_back({TokenType::CLOSE_PAREN, ")"});
        break;
      case '{':
        tokens.push_back({TokenType::OPEN_CURLY, "{"});
        break;
      case '}':
        tokens.push_back({TokenType::CLOSE_CURLY, "}"});
        break;
      case '[':
        tokens.push_back({TokenType::OPEN_BRACKET, "["});
        break;
      case ']':
        tokens.push_back({TokenType::CLOSE_BRACKET, "]"});
        break;
      case ':':
        if (peek() == '=') {
          get();
          tokens.push_back({TokenType::ASSIGNMENT, ":="});
          break;
        }
        tokens.push_back({TokenType::COLON, ":"});
        break;
      case ';':
        tokens.push_back({TokenType::SEMICOLON, ";"});
        break;
      case ',':
        tokens.push_back({TokenType::COMMA, ","});
        break;
      default:
        tokens.push_back({TokenType::UNKNOWN, std::string(1, current)});
        break;
      }
    }
  }

  tokens.push_back({TokenType::END_OF_FILE, ""});
  return tokens;
};
