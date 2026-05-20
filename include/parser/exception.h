#pragma once

#include <lexer/lexer.h>

#include <iostream>
#include <string>
#include <utility>

class ParserException : public std::exception {
  std::string message;
  Span span;
  std::string labelText;
  std::string suggestionText;

public:
  ParserException(std::string msg, Span s, std::string label = "",
                  std::string suggestion = "")
      : message(std::move(msg)), span(s), labelText(std::move(label)),
        suggestionText(std::move(suggestion)) {}

  const char *what() const noexcept override { return "Swa Syntax Error"; }
  std::string getMessage() const { return message; }
  std::string getLabelText() const { return labelText; }
  Span getSpan() const { return span; }

  // This method handles the entire Rust-style visual layout internally
  void emitDiagnostic(const SourceManager &sm) const {
    auto [line, col] = sm.getLineCol(span.start);
    auto [_, endCol] = sm.getLineCol(span.end);

    // 1. Core Header Block (Bold Red formatting)
    std::cout << "\033[1;31merror\033[0m: " << message << "\n";
    std::cout << "  --> swa_source:" << line << ":" << col << "\n";

    // 2. Code Frame Context Line
    std::cout << "   |\n";
    std::cout << " " << line << " | " << sm.getLineSnippet(line) << "\n";

    // 3. Highlight Carets (^^^^) underneath the exact bytes
    std::cout << "   | ";
    for (size_t i = 1; i < col; ++i)
      std::cout << " ";
    for (size_t i = col; i <= endCol; ++i)
      std::cout << "\033[1;31m^\033[0m";

    if (!labelText.empty()) {
      std::cout << " " << labelText;
    }
    std::cout << "\n";

    // 4. Cyan Help / Suggestion Block
    if (!suggestionText.empty()) {
      std::cout << "   | \033[1;36mhelp\033[0m: " << suggestionText << "\n";
    }
    std::cout << "\n";
  }
};
