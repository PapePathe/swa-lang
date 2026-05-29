#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: mytool <subcommand> [options]\n";
    return 1;
  }

  std::string subcommand = argv[1];
  std::string program = R"(
    dialect:english;

    start() int {
      let arr [2]int := [10, 20];
      print(arr[1]);
      return 0;
    }
  )";
  SwaCompiler swa = SwaCompiler();

  if (subcommand == "build") {
    bool release = false;
    // NOTE(pathe)
    // Start from index 2 to skip program name and subcommand
    for (int i = 2; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--release") {
        release = true;
      }
    }

    std::cout << "Building in " << (release ? "release" : "debug")
              << " mode...\n";

    swa.Build(program);

    return 0;
  } else if (subcommand == "run") {
    std::cout << "Running project...\n";
    swa.Run(program);
    return 0;
  } else if (subcommand == "test") {
    std::cout << "Testing project...\n";
    swa.Test(program);
    return 0;
  } else {
    std::cerr << "Unknown subcommand: " << subcommand << "\n";
    return 1;
  }

  return 0;
}
