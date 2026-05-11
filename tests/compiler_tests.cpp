#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

class JITOutputTest : public ::testing::Test {
protected:
  std::string runAndCapture(std::function<void()> func) {
    testing::internal::CaptureStdout();

    try {
      func();

      return testing::internal::GetCapturedStdout();
    } catch (std::runtime_error e) {
      std::cout << e.what();

      return testing::internal::GetCapturedStdout();
    }
  }
};

TEST_F(JITOutputTest, ParseMinimalProgram2) {
  std::string output = runAndCapture([&]() {
    std::string program = "main()int { print(\"1 + 3 = 4\"); }";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "1 + 3 = 4");
}

TEST_F(JITOutputTest, ParseMinimalProgram) {
  std::string output = runAndCapture([&]() {
    std::string program = "main()int { print(\"Salam\"); }";
    SwaCompiler swa = SwaCompiler();
    swa.Run(program);
  });

  ASSERT_EQ(output, "Salam");
}

TEST_F(JITOutputTest, Print) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("10 = %d", 10); 
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "10 = 10");
}

TEST_F(JITOutputTest, Print1) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("10 - 3 - 2 = %d", 10 - 3 - 2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "10 - 3 - 2 = 5");
}

TEST_F(JITOutputTest, Print2) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("16 / 4 / 2 = %d", 16 / 4 / 2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "16 / 4 / 2 = 2");
}

TEST_F(JITOutputTest, Print3) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("5 + 3 * 2 - 8 / 4 = %d", 5 + 3 * 2 - 8 / 4);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "5 + 3 * 2 - 8 / 4 = 9");
}

TEST_F(JITOutputTest, Print4) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("7 / 2 = %d", 7/2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "7 / 2 = 3");
}

TEST_F(JITOutputTest, Print5) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("12 * 3 / 4 = %d", 12 * 3 / 4);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "12 * 3 / 4 = 9");
}

TEST_F(JITOutputTest, Print6) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("5 - 3 + 2 = %d", 5 - 3 + 2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "5 - 3 + 2 = 4");
}

TEST_F(JITOutputTest, Print7) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("- 5 = %d", - 5);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "- 5 = -5");
}

TEST_F(JITOutputTest, Print8) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-(3 + 4) = %d", -(3 + 4));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-(3 + 4) = -7");
}

TEST_F(JITOutputTest, Print9) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("5 - (-3) = %d", 5 - (-3));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "5 - (-3) = 8");
}

TEST_F(JITOutputTest, Print10) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-4 * 6 = %d", -4 * 6);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-4 * 6 = -24");
}

TEST_F(JITOutputTest, Print11) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-15 / 4 = %d", -15 / 4);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-15 / 4 = -3");
}

TEST_F(JITOutputTest, Print12) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-(-5) = %d", -(-5));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-(-5) = 5");
}

TEST_F(JITOutputTest, Print13) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-7 / 2 = %d", -7 / 2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-7 / 2 = -3");
}

TEST_F(JITOutputTest, Print14) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("7 / -2 = %d", 7 / -2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "7 / -2 = -3");
}

TEST_F(JITOutputTest, Print15) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-7 / -2 = %d", -7 / -2);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-7 / -2 = 3");
}

TEST_F(JITOutputTest, Print16) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-(3 * 4) = %d", -(3 * 4));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-(3 * 4) = -12");
}

TEST_F(JITOutputTest, Print17) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("- -5 = %d", - -5);
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "- -5 = 5");
}

TEST_F(JITOutputTest, Print18) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("(-3) * (-4) = %d", (-3) * (-4));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "(-3) * (-4) = 12");
}

TEST_F(JITOutputTest, Print19) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        print_f("-(2 + 3 * 4 - 5) = %d", -(2 + 3 * 4 - 5));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "-(2 + 3 * 4 - 5) = -9");
}

TEST_F(JITOutputTest, MainReturnZero) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  // FIXME assert exit code is success
  ASSERT_EQ(output, "");
}

TEST_F(JITOutputTest, MainReturnNonZero) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        return 1;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  // FIXME assert exit code is error
  ASSERT_EQ(output, "");
}

TEST_F(JITOutputTest, MainReturnInvalidStatus) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        return 404;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  // FIXME assert exit code is error
  ASSERT_EQ(output, "");
}

TEST_F(JITOutputTest, PrintLocalSymbol) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        let x int := 3;
        print_f("x = %d", x);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3");
}

TEST_F(JITOutputTest, PrintLocalSymbolCompositeAdd) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        let x int := 3;
        let y int := x + 3;
        print_f("x = %d, y = %d", x, y);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3, y = 6");
}

TEST_F(JITOutputTest, PrintLocalSymbolCompositeSub) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        let x int := 3;
        let y int := x - 3;
        print_f("x = %d, y = %d", x, y);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3, y = 0");
}

TEST_F(JITOutputTest, PrintLocalSymbolCompositeMul) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        let x int := 3;
        let y int := x * 3;
        print_f("x = %d, y = %d", x, y);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3, y = 9");
}

TEST_F(JITOutputTest, PrintLocalSymbolCompositeDiv) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      start() int {
        let x int := 3;
        let y int := x / 3;
        print_f("x = %d, y = %d", x, y);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3, y = 1");
}

TEST_F(JITOutputTest, PrintFGlocalSymbol) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      let x int := 3;
      start() int {
        print_f("x = %d", x);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3");
}

TEST_F(JITOutputTest, PrintGlocalSymbol) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      let x int := 3;
      start() int {
        print("x =", x);
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "x = 3");
}

TEST_F(JITOutputTest, PrintManyDataTypes) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
      dialect:english;
      let x int := 3;
      start() int {
        print(true, false, x, "a string");
        return 0;
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "true false 3 a string");
}

TEST_F(JITOutputTest, PrintArgc) {
  std::string output = runAndCapture([&]() {
    std::string program = R"(
    dialect:english;
    start(argc int, argv []string) int {
      let x int := 3;
      print_f("argc = %d", argc);
    }
  )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "argc = 0");
}
