#include "parser/exception.h"
#include <lexer/lexer.h>

#include <iostream>
#include <string>

void ParserException::emitDiagnostic(const SourceManager &sm,
                                     std::string error_category) const {
  auto [line, col] = sm.getLineCol(span.start);
  auto [_, endCol] = sm.getLineCol(span.end);

  // 1. Core Header Block (Bold Red formatting)
  std::cout << "\033[1;31m" << error_category << "\033[0m: " << message << "\n";
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
