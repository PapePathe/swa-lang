#include <unistd.h>

#include <compiler/codegen.h>
#include <compiler/compiler.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

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

  void assertSwaOutput(const std::string &code, const std::string &expected) {
    testing::internal::CaptureStdout();

    try {
      SwaCompiler swa;
      swa.Run(code);
    } catch (const std::exception &e) {
      std::cout << e.what();
    }

    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected);
  }

  void assertSwaTestOutput(const std::string &code,
                           const std::string &expected) {
    testing::internal::CaptureStdout();

    try {
      SwaCompiler swa;
      swa.Test(code);
    } catch (const std::exception &e) {
      std::cout << e.what();
    }

    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, expected);
  }
};

TEST_F(JITOutputTest, ParseMinimalProgram2) {
  std::string output = runAndCapture([&]() {
    std::string program = "main()int { print(\"1 + 3 = 4\"); return 0; }";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "1 + 3 = 4");
}

TEST_F(JITOutputTest, ParseMinimalProgram) {
  std::string output = runAndCapture([&]() {
    std::string program = "main()int { print(\"Salam\"); return 0;}";
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
  std::string program = R"(
    dialect:english;
    start(argc int, argv []string) int {
      let x int := 3;
      print_f("argc = %d", argc);
      return 0;
    }
  )";

  assertSwaOutput(program, "argc = 0");
}

TEST_F(JITOutputTest, FunctionCalls1) {
  std::string program = R"(
    dialect:english;
    func my_func()int {
      return 21;
    }
    start() int {
      print("Result", my_func());
      return 0;
    }
  )";

  assertSwaOutput(program, "Result 21");
}

TEST_F(JITOutputTest, FunctionCalls2) {
  std::string program = R"(
    dialect:english;
    func add(a int, b int)int {
      return a + b;
    }
    func sub(a int, b int)int {
      return a - b;
    }
    func mul(a int, b int)int {
      return a * b;
    }
    func div(a int, b int)int {
      return a / b;
    }
    start() int {
      print("Add", add(4,2), "\n");
      print("Sub", sub(4,2), "\n");
      print("Mul", mul(4,2), "\n");
      print("Div", div(4,2), "\n");
      return 0;
    }
  )";

  assertSwaOutput(program, "Add 6 \nSub 2 \nMul 8 \nDiv 2 \n");
}

TEST_F(JITOutputTest, MissingSemiColonVarDecl) {
  std::string program = R"(
    dialect:english;
    start() int {
      let x int := 10
      return 0;
    }
  )";

  assertSwaOutput(program,
                  "\x1B[1;31merror\x1B[0m: expected variable declaration to "
                  "end with a semicolon\n  --> swa_source:4:20\n   |\n 4 |     "
                  "  let x int := 10\n   |                    "
                  "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n");
}

TEST_F(JITOutputTest, MissingSemiColonReturnStmt) {
  std::string program = R"(
    dialect:english;
    start() int {
      let x int := 10;
      return 0
    }
  )";

  assertSwaOutput(
      program, "\x1B[1;31merror\x1B[0m: return statement must be terminated by "
               "a semi colon\n  --> swa_source:5:14\n   |\n 5 |       return "
               "0\n   |              \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n   "
               "| \x1B[1;36mhelp\x1B[0m: add ; after the return statement\n\n");
}

TEST_F(JITOutputTest, MissingSemiColonPrintStmt) {
  std::string program = R"(
    dialect:english;
    start() int {
      print("test")
    }
  )";

  assertSwaOutput(
      program,
      "\x1B[1;31merror\x1B[0m: print statement must be terminated by a semi "
      "colon\n  --> swa_source:4:19\n   |\n 4 |       print(\"test\")\n   |    "
      "               \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n   | "
      "\x1B[1;36mhelp\x1B[0m: add ; after the print statement\n\n");
}

TEST_F(JITOutputTest, MissingSemiColonPrintfStmt) {
  std::string program = R"(
    dialect:english;
    start() int {
      print_f("test %d", x)
    }
  )";

  assertSwaOutput(
      program, "\x1B[1;31merror\x1B[0m: formatted print statement must be "
               "terminated by a semi colon\n  --> swa_source:4:27\n   |\n 4 |  "
               "     print_f(\"test %d\", x)\n   |                           "
               "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n   | "
               "\x1B[1;36mhelp\x1B[0m: add ; after the print_f statement\n\n");
}

TEST_F(JITOutputTest, MissingCloseParentInFunctionCall) {
  std::string program = R"(
    dialect:english;
    start() int {
      call("test %d", x;
    }
  )";

  assertSwaOutput(
      program,
      "\x1B[1;31merror\x1B[0m: unclosed function call parameter block\n  --> "
      "swa_source:4:23\n   |\n 4 |       call(\"test %d\", x;\n   |            "
      "           \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m expected a closing ')' "
      "to match the open parenthesis\n   | \x1B[1;36mhelp\x1B[0m: add a "
      "closing parenthesis to terminate the call parameters list\n\n");
}

TEST_F(JITOutputTest, FunctionCalls3) {
  std::string program = R"(
    dialect:english;
    func mul_three(a int)int {
      return 3 * a;
    }
    func mul_two(a int)int {
      return 2 * a;
    }
    func add(a int, b int)int {
      return a + b;
    }
    start() int {
      print("Result", mul_three(mul_two(add(2, 3))));
      return 0;
    }
  )";

  assertSwaOutput(program, "Result 30");
}

TEST_F(JITOutputTest, Call_Functions_That_Are_Defined_After_Main) {
  std::string program = R"(
    dialect:english;

    func add(a int, b int)int {
      return a + b;
    }

    func mul_two(a int)int {
      return 2 * a;
    }
    start() int {
      print("Result", mul_three(mul_two(add(2, 3))));
      return 0;
    }

    func mul_three(a int)int {
      return 3 * a;
    }

  )";

  assertSwaOutput(program, "Result 30");
}

// TEST_F(JITOutputTest, Call_Functions_That_Are_Defined_After_Main) {
//   std::string program = R"(
//     dialect:english;
//
//     func mul_two_add(a int, b int)int {
//       return 2 * add(a,b);
//     }
//
//     start() int {
//       print("Result", mul_three(mul_two(2, 3)));
//       return 0;
//     }
//
//     func mul_three(a int)int {
//       return 3 * a;
//     }
//
//     func add(a int, b int)int {
//       return a + b;
//     }
//   )";
//
//   assertSwaOutput(program, "Result 30");
// }

TEST_F(JITOutputTest, TestFrameworkAssertEqual) {
  std::string program = R"(
    dialect:english;

    func add_not_impl(a int, b int)int {
      return -1;
    }

    func add(a int, b int)int {
      return a + b;
    }

    test "assert equal success" {
      let result int := add(5, 5);

      assert_equal result, 10;
      assert_equal result, 11;
    }
  )";

  assertSwaTestOutput(
      program,
      "[RUN] swa_test_assert_equal_success\n  -> \x1B[31m[FAIL]\x1B[0m "
      "Expected 11, but got 10\n  -> \x1B[32m[PASS]\x1B[0m\n");
}
