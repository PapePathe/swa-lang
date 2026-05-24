#pragma once

#include <assert.h>
#include <lexer/tokentype.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Location {
  size_t offset = 0;
};

struct Span {
  Location start;
  Location end;
  Span() = default;
  Span(const Span &) = default;
  Span(Location s, Location e) : start{s}, end{e} {}
  static Span merge(Span a, Span b) { return Span(a.start, b.end); }
};

struct Token {
  TokenType type;
  std::string value;
  Span span;

  Token(TokenType t, std::string v, Location s, Location e)
      : type(t), value(std::move(v)), span(s, e) {}
};

class SourceManager {
  std::string source;
  std::vector<size_t> lineOffsets;

public:
  explicit SourceManager(std::string src) : source(std::move(src)) {
    lineOffsets.push_back(0);
    for (size_t i = 0; i < source.size(); ++i) {
      if (source[i] == '\n') {
        lineOffsets.push_back(i + 1);
      }
    }
  }

  struct LineCol {
    size_t line;
    size_t col;
  };

  LineCol getLineCol(Location loc) const {
    auto it =
        std::upper_bound(lineOffsets.begin(), lineOffsets.end(), loc.offset);
    size_t lineIdx = std::distance(lineOffsets.begin(), it) - 1;
    size_t colIdx = loc.offset - lineOffsets[lineIdx] + 1;
    return {lineIdx + 1, colIdx};
  }

  std::string getLineSnippet(size_t lineNum) const {
    if (lineNum == 0 || lineNum > lineOffsets.size())
      return "";
    size_t start = lineOffsets[lineNum - 1];
    size_t end = (lineNum < lineOffsets.size()) ? lineOffsets[lineNum] - 1
                                                : source.size();
    return source.substr(start, end - start);
  }
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
