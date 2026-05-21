#ifndef SWA_LEXER
#define SWA_LEXER

#include <assert.h>
#include <lexer/tokentype.h>

#include <algorithm>
#include <iostream>
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

  static Span merge(Span a, Span b) { return {a.start, b.end}; }
};

struct Token {
  TokenType type;
  std::string value;
  Span span;
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

// enum class DiagnosticLevel { Error, Warning, Note };
//
// class DiagnosticBuilder {
//   const SourceManager &sm;
//   DiagnosticLevel level;
//   std::string message;
//   Span primarySpan;
//   std::string labelText;
//   std::string suggestionText;
//
// public:
//   DiagnosticBuilder(const SourceManager &sm, DiagnosticLevel lvl,
//                     std::string msg, Span span)
//       : sm(sm), level(lvl), message(std::move(msg)), primarySpan(span) {}
//
//   DiagnosticBuilder &withLabel(std::string text) {
//     labelText = std::move(text);
//     return *this;
//   }
//
//   DiagnosticBuilder &withSuggestion(std::string text) {
//     suggestionText = std::move(text);
//     return *this;
//   }
//
//   void emit() {
//     auto [line, col] = sm.getLineCol(primarySpan.start);
//     auto [_, endCol] = sm.getLineCol(primarySpan.end);
//
//     // 1. Print header (Color codes: \033[1;31m is Bold Red)
//     std::cerr << "\033[1;31merror\033[0m: " << message << "\n";
//     std::cerr << "  --> swa_program:" << line << ":" << col << "\n";
//
//     // 2. Print code snippet
//     std::cerr << "   |\n";
//     std::cerr << " " << line << " | " << sm.getLineSnippet(line) << "\n";
//
//     // 3. Print underline caret (^)
//     std::cerr << "   | ";
//     for (size_t i = 1; i < col; ++i)
//       std::cerr << " ";
//     for (size_t i = col; i <= endCol; ++i)
//       std::cerr << "\033[1;31m^\033[0m";
//
//     if (!labelText.empty()) {
//       std::cerr << " " << labelText;
//     }
//     std::cerr << "\n";
//
//     // 4. Print suggestion if available
//     if (!suggestionText.empty()) {
//       std::cerr << "   | \033[1;36mhelp\033[0m: " << suggestionText << "\n";
//     }
//     std::cerr << "\n";
//   }
// };
#endif // !SWA_LEXER
