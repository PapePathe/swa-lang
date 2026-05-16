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

    func add(a int, b int) int {
      return a + b;
    }

    test "math addition operations" {
      let result int := add(5, 5);
      assert_equal result, 10;
      assert_equal add(15, 5), 20;
    }

    test "intentional assertion failure" {
      let broken int := add(2, 2);
      assert_equal broken, 5;
    }

    start()int {
      print("Salam");
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
