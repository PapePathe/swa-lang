#pragma once

#include <lexer/lexer.h>

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

  void emitDiagnostic(const SourceManager &sm,
                      std::string error_category) const;
};
