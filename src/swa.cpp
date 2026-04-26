#include "../include/compiler/compiler.hpp"
#include <string>

int main() {
  std::string program = R"(
    dialect:english;
    main() int { 
      print("Nangadef Pathe");
    }
  )";
  SwaCompiler swa = SwaCompiler();
  swa.Run(program);

  return 0;
}
