#include <compiler/compiler.h>
#include <string>

int main(int argc, char *argv[]) {
  std::string program = R"(
    dialect:english;
    start() int {
      let x int := 3;
      print("x = %d", x);
    }
  )";
  SwaCompiler swa = SwaCompiler();
  if (argc < 2) {
    std::cerr << "Usage: mytool <subcommand> [options]\n";
    return 1;
  }

  std::string subcommand = argv[1];

  if (subcommand == "build") {
    bool release = false;
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
  } else if (subcommand == "run") {
    std::cout << "Running project...\n";
    swa.Run(program);
  } else {
    std::cerr << "Unknown subcommand: " << subcommand << "\n";
    return 1;
  }

  return 0;
}
