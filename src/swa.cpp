#include <iostream>
#include <string>
#include "../include/compiler/compiler.hpp"

int main() {
  std::string program = R"(
        retourner 0;
  )";

  SwaCompiler swa = SwaCompiler();

  swa.Run(program);

  return 0;
}
