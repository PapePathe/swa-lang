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

TEST_F(JITOutputTest, Call_Function_That_Does_Not_Exist) {
  std::string program = R"(
    dialect:english;
    start() int {
      print("Result", my_func());
      return 0;
    }
  )";

  std::string expected_diagnostic =
      "\x1B[1;31mcode generation error\x1B[0m: Function named my_func does not "
      "exist\n  --> swa_source:4:23\n   |\n 4 |       print(\"Result\", "
      "my_func());\n   |                       "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n";

  assertSwaOutput(program, expected_diagnostic);
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

TEST_F(JITOutputTest, Call_Inner_Functions_That_Are_Defined_After_Main) {
  std::string program = R"(
     dialect:english;

     func mul_two_add(a int, b int)int {
       return 2 * add(a,b);
     }

     start() int {
       print("Result", mul_three(mul_two_add(2, 3)));
       return 0;
     }

     func mul_three(a int)int {
       return 3 * a;
     }

     func add(a int, b int)int {
       return a + b;
     }
   )";

  assertSwaOutput(program, "Result 30");
}

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

TEST_F(JITOutputTest, Self_Referencing_Struct) {
  std::string program = R"(
    dialect:english;
    struct Node {
      value int,
      prev *Node,
      next *Node,
    }
    func new_node(value int, left Node, right Node) int {
      return 0;
    }
    start() int {
      let n Node;
      print("TODO");
      return 0;
    }
  )";

  assertSwaOutput(program, "TODO");
}

TEST_F(JITOutputTest, Self_Referencing_Struct_Without_pointer) {
  std::string program = R"(
     dialect:english;
     struct Node {
       value int,
       prev Node,
       next Node,
     }
     start() int {
      let n Node;
       return 0;
     }
   )";

  assertSwaOutput(
      program,
      "\x1B[1;31mcode generation error\x1B[0m: Compile Error: Invalid "
      "recursive type. Struct 'Node' cannot directly contain itself without a "
      "pointer.\n  --> swa_source:3:6\n   |\n 3 |      struct Node {\n   |     "
      " \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n");
}

TEST_F(JITOutputTest, Function_Parameter_Struct_by_Pointer) {
  std::string program = R"(
    dialect:english;
    struct Node {
      value int,
    }
    func new_node(value int, left *Node, right *Node) int {
      return 0;
    }
    start() int {
      let n Node;
      print("TODO");
      return 0;
    }
  )";

  assertSwaOutput(program, "TODO");
}

TEST_F(JITOutputTest, Reference_To_Undefined_Struct) {
  std::string program = R"(
    dialect:english;
    func new_node(value int, left Node, right Node) int {
      return 0;
    }
    start() int {
      return 0;
    }
  )";

  std::string expected_diagnostic =
      "\x1B[1;31mcode generation error\x1B[0m: Undefined struct Node\n  --> "
      "swa_source:3:35\n   |\n 3 |     func new_node(value int, left Node, "
      "right Node) int {\n   |                                   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\n\n";
  assertSwaOutput(program, expected_diagnostic);
}

TEST_F(JITOutputTest, ThrowsErrorOnInvalidMainSignature) {
  std::string input = R"(dialect:english;
    start(argc int, argv int) int {
      return 0;
    }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: second argument of main should "
      "be a slice of strings\n  --> swa_source:2:26\n   |\n 2 |     start(argc "
      "int, argv int) int {\n   |                          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\n\n";

  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, ThrowsErrorOnInvalidMainSignature2) {
  std::string input = R"(dialect:english;
    start(argc string) int {
      return 0;
    }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: first argument of main should "
      "be of TypeInt\n  --> swa_source:2:16\n   |\n 2 |     start(argc string) "
      "int {\n   |                "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n";

  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, ThrowsErrorOnInvalidMainSignature3) {
  std::string input = R"(dialect:english;
    start(a int, b []string, c string) int {
      return 0;
    }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: third argument of main should "
      "be a slice of strings\n  --> swa_source:2:32\n   |\n 2 |     start(a "
      "int, b []string, c string) int {\n   |                                "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n";

  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, ThrowsErrorOnInvalidMainSignature4) {
  std::string input = R"(dialect:english;
    start(a int, b []string, c []string, d int) int {
      return 0;
    }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: main should have at most 3 "
      "arguments\n  --> swa_source:2:5\n   |\n 2 |     start(a int, b "
      "[]string, c []string, d int) int {\n   |     "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\n\n";

  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, ThrowsErrorOnInvalidMainSignature5) {
  std::string input = R"(dialect:english;
    start() string{
      return 0;
    }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: return value of main should be "
      "TypeInt\n  --> swa_source:2:13\n   |\n 2 |     start() string{\n   |    "
      "         "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n";

  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_AddIntString) {
  std::string input = "let a int := 10 + \"10\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  --> "
      "swa_source:1:14\n   |\n 1 | let a int := 10 + \"10\";\n   |             "
      " \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_NestedMathMismatch) {
  std::string input = "let x int := (10 + (20 + (30 + \"40\")));";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  --> "
      "swa_source:1:26\n   |\n 1 | let x int := (10 + (20 + (30 + \"40\")));\n "
      "  |                          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m Cannot '+' variable of type Int with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_AddBooleans) {
  std::string input = "let a bool := true + false;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type Error\n  --> swa_source:1:15\n  "
      " |\n 1 | let a bool := true + false;\n   |               "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m The '+'  operator cannot be applied to type "
      "'Bool'.\n   | \x1B[1;36mhelp\x1B[0m: Consider using logical operators "
      "like '&&' or '||' if you intended to perform a logical "
      "operation.\n\n\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  "
      "--> swa_source:1:15\n   |\n 1 | let a bool := true + false;\n   |       "
      "        "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Expected Bool but got Int\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_AssignStringToInt) {
  std::string input = "let a int := \"hello world\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:1:14\n   |\n 1 | let a int := \"hello world\";\n   |         "
      "     "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected Int but got String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_AddStringsNotSupported) {
  std::string input = "let a string := \"hello\" + \"world\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Invalid Operation\n  --> "
      "swa_source:1:17\n   |\n 1 | let a string := \"hello\" + \"world\";\n   "
      "|                 "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Operator '+' is not supported "
      "for types 'String' and 'String'.\n   | \x1B[1;36mhelp\x1B[0m: Check "
      "your types or use the appropriate library function for this "
      "operation.\n\n\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  "
      "--> swa_source:1:17\n   |\n 1 | let a string := \"hello\" + "
      "\"world\";\n   |                 "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected String but got Int\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_VariableTypeMismatch) {
  std::string input = "let x int := 10; let y string := x + 5;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:1:34\n   |\n 1 | let x int := 10; let y string := x + 5;\n   "
      "|                                  "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected String but got Int\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_IntMultiplyString) {
  std::string input = "let x int := 10 * \"5\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in division \n  --> "
      "swa_source:1:14\n   |\n 1 | let x int := 10 * \"5\";\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '*' variable of type Int with a value of "
      "type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_BoolDivideInt) {
  std::string input = "let x int := true / 2;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in  \n  --> "
      "swa_source:1:14\n   |\n 1 | let x int := true / 2;\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '/' variable of type Bool with a value of "
      "type Int\n\n\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:1:14\n   |\n 1 | let x int := true / 2;\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Expected Int but got Bool\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_ComplexNestedArithmeticMismatch) {
  std::string input = "let x int := (5 * (10 - 2)) + (\"invalid\" / 2);";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in  \n  --> "
      "swa_source:1:31\n   |\n 1 | let x int := (5 * (10 - 2)) + (\"invalid\" "
      "/ 2);\n   |                               "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m Cannot '/' variable of type String with a value of type "
      "Int\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  "
      "--> swa_source:1:14\n   |\n 1 | let x int := (5 * (10 - 2)) + "
      "(\"invalid\" / 2);\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int with a value of "
      "type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_AssignBoolToString) {
  std::string input = "let x string := true;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:1:17\n   |\n 1 | let x string := true;\n   |                 "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Expected String but got Bool\n\n";
  assertSwaOutput(input, expected_diagnostic);
}
// Operator Precedence/Grouping Errors
// Tests if the compiler correctly types the intermediate result of a
// sub-expression
TEST_F(JITOutputTest, Error_OperatorPrecedenceMismatch) {
  // Programmer expects 10 + 20 then * "3", compiler should catch String type in
  // multiplication
  std::string input = "let x int := 10 + 20 * \"3\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in division "
      "\n  --> swa_source:1:19\n   |\n 1 | let x int := 10 + 20 * "
      "\"3\";\n   |                   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;"
      "31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^"
      "\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '*' "
      "variable of type Int with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

// Type "Shadowing" / Variable Drift
// Tests if the compiler resolves the type of a variable correctly within a
// nested scope
TEST_F(JITOutputTest, Error_VariableTypeDrift) {
  std::string input =
      "let x int := 10; { let x string := \"hello\"; let y int := x + 5; }";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  --> "
      "swa_source:1:58\n   |\n 1 | let x int := 10; { let x string := "
      "\"hello\"; let y int := x + 5; }\n   |                                  "
      "                        "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type "
      "String with a value of type Int\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

// Chain of Operation Failure
// Errors often occur at the "leaf" of a complex tree; this ensures the error
// bubbles up
TEST_F(JITOutputTest, Error_DeepExpressionTypeCollision) {
  std::string input = "let a int := 1; let b int := 2; let c int := (a + (b * "
                      "(10 / (a + \"str\"))));";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  --> "
      "swa_source:1:62\n   |\n 1 | let a int := 1; let b int := 2; let c int "
      ":= (a + (b * (10 / (a + \"str\"))));\n   |                              "
      "                                "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m Cannot '+' variable of type Int with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

// Mixed Arithmetic/Boolean Logic
// A classic "logical error" where a user mixes up numeric comparison and
// boolean logic
TEST_F(JITOutputTest, Error_BooleanNumericMixedOp) {
  std::string input =
      "let a int := 5; let b bool := true; let res int := a + (b * 10);";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in division \n  --> "
      "swa_source:1:56\n   |\n 1 | let a int := 5; let b bool := true; let res "
      "int := a + (b * 10);\n   |                                              "
      "          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '*' variable of type Bool with a value of "
      "type Int\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in "
      "addition \n  --> swa_source:1:52\n   |\n 1 | let a int := 5; let b bool "
      ":= true; let res int := a + (b * 10);\n   |                             "
      "                       "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int with a value of "
      "type Bool\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

//  Implicit Type Conversion Attempt
// Modern compilers are strict; this tests the rejection of "loose" coding
// styles
TEST_F(JITOutputTest, Error_StringAdditionAttempt) {
  std::string input = "let a string := \"val\" + 5;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  --> "
      "swa_source:1:17\n   |\n 1 | let a string := \"val\" + 5;\n   |          "
      "       "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type "
      "String with a value of type Int\n\n\x1B[1;31mtype check error\x1B[0m: "
      "Incompatible type\n  --> swa_source:1:17\n   |\n 1 | let a string := "
      "\"val\" + 5;\n   |                 "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected String but got Int\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_MultiplyStringWithInt) {
  std::string input = "let x string := \"hello\"; let y int := x * 5;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in division \n  --> "
      "swa_source:1:39\n   |\n 1 | let x string := \"hello\"; let y int := x * "
      "5;\n   |                                       "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '*' variable of type "
      "String with a value of type Int\n\n\x1B[1;31mtype check error\x1B[0m: "
      "Incompatible type\n  --> swa_source:1:39\n   |\n 1 | let x string := "
      "\"hello\"; let y int := x * 5;\n   |                                    "
      "   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected Int but got String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_DivideBoolWithBool) {
  std::string input = "let x bool := true / false;";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type Error\n  --> swa_source:1:15\n  "
      " |\n 1 | let x bool := true / false;\n   |               "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m The '/'  operator cannot be applied to type "
      "'Bool'.\n   | \x1B[1;36mhelp\x1B[0m: Consider using logical operators "
      "like '&&' or '||' if you intended to perform a logical operation.\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_SubtractIntAndString) {
  std::string input = "let x int := 10 - \"5\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in subtraction \n  --> "
      "swa_source:1:14\n   |\n 1 | let x int := 10 - \"5\";\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '-' variable of type Int with a value of "
      "type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_DeclarationMismatch) {
  std::string input = "let x int := \"not an int\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:1:14\n   |\n 1 | let x int := \"not an int\";\n   |          "
      "    "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Expected Int but got String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_ComplexArithmeticMixingTypes) {
  std::string input =
      "let x int := 10; let y int := 20; let z int := (x + y) - \"string\";";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in subtraction \n  --> "
      "swa_source:1:48\n   |\n 1 | let x int := 10; let y int := 20; let z int "
      ":= (x + y) - \"string\";\n   |                                          "
      "      "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '-' "
      "variable of type Int with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Valid_BasicArithmetic) {
  std::string input =
      "dialect:english; start() int{let x int := 10 + 20 * 5;return 0;}";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Valid_NestedArithmetic) {
  std::string input = "dialect:english; start() int{let x int := (10 + 20) * "
                      "(30 - 5);return 0;}";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Valid_VariableReassignmentInBlock) {
  // Tests that scoping works and doesn't trigger false error on shadowing
  std::string input = "dialect:english; start() int{let x int := 10; { let x "
                      "int := 20; let y int := x + 5; }return 0;}";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Valid_ComplexArithmeticWithVariables) {
  std::string input = "dialect:english; start() int{let a int := 1; let b int "
                      ":= 2; let c int := 3; "
                      "let res int := a + (b * c) - (a / b);return 0;}";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Valid_DeeplyNestedValidArithmetic) {
  std::string input = "dialect:english; start() int{let res int := (1 + (2 * "
                      "(3 - (4 / 2))));return 0;}";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Valid_MultipleDeclarations) {
  std::string input = "dialect:english; start() int{let a int := 10; let b int "
                      ":= a + 5; let c int := b * 2;return 0;}";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Program_Valid_ScopeIsolation) {
  std::string input = R"(
        dialect:english;
        start() int {
            let x int := 10;
            {
                let x int := 20;
                let y int := x + 5;
            }
            let z int := x + 5;
            return 0;
        }
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Program_Valid_OperatorPrecedence) {
  std::string input = R"(
        dialect:english;
        start() int {
            let a int := 10 + 5 * 2;
            let b int := (10 + 5) * 2;
            let c int := a + b * 0;
            return 0;
        }
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Program_Valid_DeeplyNestedArithmetic) {
  std::string input = R"(
        dialect:english;
        start() int {
            let a int := 1;
            let b int := 2;
            let c int := 3;
            let d int := (a + (b * (c - (a + b))));
            return 0;
        }
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Program_Error_ScopeBoundaryViolation) {
  std::string input = R"(
        dialect:english;
        start() int {
            let x int := 10;
            {
                let y int := 20;
            }
            let z int := y + 5;
            return 0;
        }
    )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: variable y does not exist\n  --> "
      "swa_source:8:26\n   |\n 8 |             let z int := y + 5;\n   |       "
      "                   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n\x1B[1;31mtype check "
      "error\x1B[0m: Type mismatch in addition \n  --> swa_source:8:26\n   |\n "
      "8 |             let z int := y + 5;\n   |                          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Void "
      "with a value of type Int\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Program_Error_TypeMismatchInExpression) {
  std::string input = R"(
        dialect:english;
        start() int {
            let a int := 10;
            let b string := "20";
            let c int := a + b;
            return 0;
        }
    )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition \n  --> "
      "swa_source:6:26\n   |\n 6 |             let c int := a + b;\n   |       "
      "                   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Program_Error_ComplexNestedMismatch) {
  std::string input = R"(
        dialect:english;
        start() int {
            let a int := 1;
            let b int := 2;
            let c int := (a * b) + (a - "invalid");
            return 0;
        }
    )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in subtraction \n  --> "
      "swa_source:6:36\n   |\n 6 |             let c int := (a * b) + (a - "
      "\"invalid\");\n   |                                    "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m Cannot '-' variable of type Int with a value of type String\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

// Ensures that shadowing a variable in an inner block does not break the
// outer variable's integrity once the block scope is popped.
TEST_F(JITOutputTest, Program_Scope_ShadowingIntegrity) {
  std::string input = R"(
dialect:english;
start() int {
    let x int := 10;
    {
        let x int := 20;
        let y int := x + 5;
    }
    let z int := x + 5;
    return 0;
}
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

// Tests if the TypeChecker correctly evaluates types based on tree structure
// rather than linear order. 10 + 5 * 2 should be parsed as 10 + (5 * 2).
TEST_F(JITOutputTest, Program_Precedence_StandardOrder) {
  std::string input = R"(
dialect:english;
start() int {
    let val int := 10 + 5 * 2;
    let check int := 20;
    return 0;
}
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

// Stress-tests the Visitor's ability to propagate types up from a deep leaf
// to a root node without dropping or mislabeling the datatype.
TEST_F(JITOutputTest, Program_Boundary_DeepArithmeticLeaf) {
  std::string input = R"(
dialect:english;
start() int {
    let a int := 1;
    let b int := (a + (a + (a + (a + (a + (a + (a + (a + 1))))))));
    return 0;
}
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

// A common real-world bug: attempting to access a variable initialized only
// within an inner block scope.
TEST_F(JITOutputTest, Program_Error_ScopeLeakage) {
  std::string input = R"(
dialect:english;
start() int {
    let a int := 10;
    {
        let b int := 20;
    }
    let c int := a + b;
    return 0;
}
    )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: variable b does not exist\n  --> "
      "swa_source:8:22\n   |\n 8 |     let c int := a + b;\n   |               "
      "       \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n\x1B[1;31mtype check "
      "error\x1B[0m: Type mismatch in addition \n  --> swa_source:8:18\n   |\n "
      "8 |     let c int := a + b;\n   |                  "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type Void\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

// Explicitly forces the parser to ignore standard precedence, checking if the
// visitor correctly respects the AST node priority.
TEST_F(JITOutputTest, Program_Precedence_GroupingOverride) {
  std::string input = R"(
dialect:english;
start() int {
    let result int := (10 + 5) * 2;
    return 0;
}
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

//  Error: Comparing String and Int
// A classic error: programmers often forget to parse a string before comparing
// it to an int.
TEST_F(JITOutputTest, Error_CompareStringAndInt) {
  std::string input = R"(
        dialect:english;
        start() int {
            let x int := 10;
            let y string := "10";
            if (x = y) { return 1; }
            return 0;
        }
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

//  Error: Using comparison operators on Booleans
// While some languages allow this (false < true), strict compilers often
// prevent it.
TEST_F(JITOutputTest, Error_CompareBooleansWithOperators) {
  std::string input = R"(
        dialect:english;
        start() int {
            let a bool := true;
            let b bool := false;
            if (a > b) { return 1; }
            return 0;
        }
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

//  Error: Comparing result of a math expression to a String
TEST_F(JITOutputTest, Error_CompareMathResultToString) {
  std::string input = R"(
        dialect:english;
        start() int {
            let x int := 5 + 5;
            if (x < "10") { return 1; }
            return 0;
        }
    )";
  std::string expected_diagnostic = "";
  assertSwaOutput(input, expected_diagnostic);
}

//  Error: String inequality check
// Ensure that the compiler rejects comparison between incompatible types even
// in inequality.
// TEST_F(JITOutputTest, Error_InequalityStringBool) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let a string := "hello";
//             let b bool := true;
//             if (a != b) { return 1; }
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_AndBoolInt) {
//  std::string input = "let a bool := true && 1;";
//  std::string expected_diagnostic = "";
//  assertSwaOutput(input, expected_diagnostic);
//}
// FIXME
// TEST_F(JITOutputTest, Error_CompareIntString) {
//   std::string input = "let a bool := (10 + 20) < \"30\";";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_ComplexLogicMismatch) {
//   std::string input = "let x bool := (10 < 20) + (true && false);";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_StringModuloInt) {
//   std::string input = "let x int := \"abc\" % 3;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// FIXME
// TEST_F(JITOutputTest, Error_OrStringBool) {
//   std::string input = "let x bool := \"true\" || true;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_GreaterEqualStringInt) {
//   std::string input = "let x bool := \"10\" >= 10;";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_NotString) {
//   std::string input = "let x bool := !\"hello\";";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_MixedComparisonChain) {
//   std::string input = "let x bool := 10 == \"10\";";
//  std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

//  Invalid Identity/Self-Assignment
// Ensuring the compiler prevents operating on an uninitialized or wrong-type
// self-reference
// TEST_F(JITOutputTest, Error_CircularTypeDependency) {
//   std::string input = "let x int := x + 1;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Error_UndefinedVariable) {
//   std::string input = "let a int := b + 10;";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// Tests the boundary between the SymbolTable declaration phase and
// the type-checking phase for re-assignment.
// TEST_F(JITOutputTest, Program_Boundary_SelfAssignment) {
//   std::string input = R"(
// dialect:english;
// start() int {
//     let x int := 10;
//     x := x + 10;
//     return 0;
// }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// TEST_F(JITOutputTest, Program_Valid_CallWithDefinitions) {
//   std::string input = R"(
//         dialect:english;
//
//         add(a int, b int) int {
//             return a + b;
//         }
//
//         start() int {
//             let res int := add(10, 20);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// TEST_F(JITOutputTest, Program_Error_CallWithMissingDefinition) {
//   std::string input = R"(
//         dialect:english;
//
//         start() int {
//             let res int := unknown_function(10);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// TEST_F(JITOutputTest, Error_CallArgMismatchInDefinition) {
//   std::string input = R"(
//         dialect:english;
//
//         multiply(a int, b int) int { return a * b; }
//
//         start() int {
//             // Error: multiply expects 2 arguments, only 1 provided
//             let res int := multiply(5);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// TEST_F(JITOutputTest, Error_CallTypeMismatchInDefinition) {
//   std::string input = R"(
//         dialect:english;
//
//         show(msg string) int { return 0; }
//
//         start() int {
//             // Error: show expects string, got int
//             let res int := show(100);
//             return 0;
//         }
//     )";
////   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }

// //  Error: Argument count mismatch (too few)
// TEST_F(JITOutputTest, Error_Call_TooFewArguments) {
//   std::string input = R"(
//         dialect:english;
//         func calculate(a int, b int) int { return a + b; }
//         start() int {
//             let res int := calculate(10);
//             return 0;
//         }
//     )";
//   std::string expected_diagnostic = "";
//   assertSwaOutput(input, expected_diagnostic);
// }
//
// //  Error: Argument count mismatch (too many)
// TEST_F(JITOutputTest, Error_Call_TooManyArguments) {
//   std::string input = R"(
//         dialect:english;
//         func calculate(a int, b int) int { return a + b; }
//         start() int {
//             let res int := calculate(10, 20, 30);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Argument count mismatch for 'calculate'");
// }
//
// //  Error: Argument type mismatch
// TEST_F(JITOutputTest, Error_Call_ArgumentTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func log(msg string) int { return 0; }
//         start() int {
//             let res int := log(42);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Type mismatch in argument for 'log'");
// }
//
// //  Error: Type mismatch in nested function call
// TEST_F(JITOutputTest, Error_Call_NestedArgumentMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func get_name() string { return "test"; }
//         func process(val int) int { return val; }
//         start() int {
//             let res int := process(get_name());
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Type mismatch in argument for 'process'");
// }
//
// //  Error: Function return type mismatch in assignment
// TEST_F(JITOutputTest, Error_Call_AssignmentTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func get_val() int { return 10; }
//         start() int {
//             let res string := get_val();
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot assign Int to variable of type
//   String");
// }
//
// // 7. Error: Calling a function with an invalid expression as an argument
// TEST_F(JITOutputTest, Error_Call_InvalidExpressionArgument) {
//   std::string input = R"(
//         dialect:english;
//         func process(val int) int { return val; }
//         start() int {
//             let res int := process("invalid" + 10);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on String and Int");
// }
// //  Error: Function call on a non-callable identifier
// // Prevents treating a variable as a function (common in languages with
// // first-class functions)
// TEST_F(JITOutputTest, Error_Call_IdentifierIsNotFunction) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let x int := 10;
//             let y int := x(5); // Error: x is a variable, not a function
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Identifier 'x' is not a function");
// }
//
// //  Error: Shadowing an existing function with a variable
// // Ensures the compiler detects name collisions between scopes and function
// // tables
// TEST_F(JITOutputTest, Error_Call_ShadowedFunction) {
//   std::string input = R"(
//         dialect:english;
//         func foo() int { return 1; }
//         start() int {
//             let foo int := 10;
//             return foo(5); // Error: local 'foo' shadows function 'foo'
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Identifier 'foo' is a variable, not a function");
// }
//
// //  Error: Recursive call with incorrect return type propagation
// // Validates that the return type is checked against the *declaration*, not
// the
// // incomplete body
// TEST_F(JITOutputTest, Error_Call_RecursiveReturnTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func recursive(n int) int {
//             return recursive(n) + "wrong"; // Error: recursive returns int,
//             not string
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on Int and String");
// }
//
// //  Error: Argument expression with side effects/type drift
// // Verifies that type inference doesn't drift when multiple operations are
// // involved
// TEST_F(JITOutputTest, Error_Call_ComplexTypeDrift) {
//   std::string input = R"(
//         dialect:english;
//         func compute(a int) int { return a; }
//         start() int {
//             // Error: (10 + "20") is a type error, regardless of the function
//             call let res int := compute(10 + "20"); return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on Int and String");
// }
//
// //  Error: Attempting to call a function defined in a deeper scope
// // Validates that lexical scoping prevents calling functions defined inside
// // other blocks
// TEST_F(JITOutputTest, Error_Call_ScopeVisibility) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             {
//                 func hidden() int { return 1; }
//             }
//             return hidden(); // Error: 'hidden' is not in the global/parent
//             scope
//         }
//     )";
//   assertSwaOutput(input, "Error: Function 'hidden' is undefined");
// }
//
// //  Error: Recursive depth/Invalid base case logic
// // Verifies that the compiler enforces return type consistency across all
// // branches
// TEST_F(JITOutputTest, Error_Call_InconsistentReturnTypes) {
//   std::string input = R"(
//         dialect:english;
//         func test(a int) int {
//             if (a == 0) { return "oops"; } // Error: returns string, expected
//             int return a;
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Function 'test' returns String, expected
//   Int");
// }
//
//  Error: Identifier name collision between parameter and function
// // Validates that parameter names do not accidentally "hijack" function
// // resolution.
// TEST_F(JITOutputTest, Error_Call_ParameterShadowsFunction) {
//   std::string input = R"(
//         dialect:english;
//         func calculate(a int) int { return a; }
//         func test(calculate int) int {
//             // 'calculate' here refers to the parameter, not the function.
//             // Attempting to invoke the parameter as a function should error.
//             return calculate(10);
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(
//       input, "Error: Identifier 'calculate' is a variable, not a function");
// }
//
// //  Error: Void-like function return value assignment
// // If your language supports functions that don't return values (or you plan
// // to), ensure you cannot assign a 'void' result to a typed variable.
// TEST_F(JITOutputTest, Error_Call_AssignVoidToVariable) {
//   std::string input = R"(
//         dialect:english;
//         func procedure() { /* No return type */ }
//         start() int {
//             let x int := procedure();
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Function 'procedure' does not return a
//   value");
// }
//
// //  Error: Circular dependency in type inference
// // This checks for the "infinite loop" potential when an argument's type
// depends
// // on the function call itself, which in turn depends on the argument.
// TEST_F(JITOutputTest, Error_Call_CircularDependency) {
//   std::string input = R"(
//         dialect:english;
//         func identity(a int) int { return a; }
//         start() int {
//             let x int := identity(x + 1);
//             return 0;
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Recursive dependency in type inference for 'x'");
// }
//
// //  Error: Calling a function within its own default parameter (if you
// expand
// // syntax) Or simply passing an uninitialized variable into a function.
// TEST_F(JITOutputTest, Error_Call_UninitializedArgument) {
//   std::string input = R"(
//         dialect:english;
//         func process(a int) int { return a; }
//         start() int {
//             let x int; // Uninitialized
//             return process(x);
//         }
//     )";
//   assertSwaOutput(input, "Error: Variable 'x' used before initialization");
// }
//
// //  Error: Expression result as function name
// // Modern parsers should prevent an expression (like a variable or logic)
// from
// // being evaluated as a function pointer unless the language explicitly
// supports
// // it.
// TEST_F(JITOutputTest, Error_Call_ExpressionAsCallee) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let op string := "add";
//             return op(10, 20); // Error: op is a string expression, not a
//             function identifier
//         }
//     )";
//   assertSwaOutput(input, "Error: 'op' is not a callable function");
// }
//
// //  Error: Argument count mismatch with variadic-like signatures
// // If your language doesn't support them, ensure it explicitly rejects
// trailing
// // commas or extra arguments that mimic variadic calls.
// TEST_F(JITOutputTest, Error_Call_TrailingCommaMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func add(a int, b int) int { return a + b; }
//         start() int {
//             // Parsing error or semantic error for dangling argument/trailing
//             comma return add(10, 20, );
//         }
//     )";
//   assertSwaOutput(input, "Error: Unexpected token in argument list");
// }
//
//  Error: Function call on a primitive literal
// // Prevents treating a literal value as a function, similar to GCC's
// diagnostic
// // when someone writes 10(5) instead of 10 * 5.
// TEST_F(JITOutputTest, Error_Call_PrimitiveLiteralAsFunction) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             let x int := 10(5); // Error: 10 is an Int, not a function
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: '10' is not a function");
// }
//
// //  Error: Argument expression is a function that returns void
// // Ensures that you cannot use the return value of a void function in an
// // argument expression.
// TEST_F(JITOutputTest, Error_Call_PassingVoidResult) {
//   std::string input = R"(
//         dialect:english;
//         func do_nothing() { }
//         func take_int(a int) int { return a; }
//         start() int {
//             let x int := take_int(do_nothing()); // Error: do_nothing returns
//             nothing return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot pass void result to function");
// }
//
// //  Error: Function call name collision with built-in or keyword (if
// // applicable) Tests if your scope resolution correctly prioritizes
// user-defined
// // functions over local variables that share names.
// TEST_F(JITOutputTest, Error_Call_NameCollisionWithLocal) {
//   std::string input = R"(
//         dialect:english;
//         func test() int { return 1; }
//         start() int {
//             let test int := 10;
//             // The compiler must resolve 'test' to the Int variable, not the
//             function return test(5);
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Identifier 'test' is a variable, not a function");
// }
// //  Error: Type mismatch via implicit narrowing (if you support Float/Int)
// // If you eventually support multiple numeric types, ensure you don't allow
// // implicit narrowing if your language policy is strict.
// TEST_F(JITOutputTest, Error_Call_NarrowingConversion) {
//   std::string input = R"(
//         dialect:english;
//         func accept_int(a int) int { return a; }
//         start() int {
//             let x float := 10.5;
//             let y int := accept_int(x); // Error: Implicit float to int
//             narrowing return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Cannot pass Float to function expecting
//   Int");
// }
// //  Error: Recursive call in global scope (if prohibited)
// // Some compilers strictly enforce that global-level expressions must be
// // constants.
// TEST_F(JITOutputTest, Error_Call_IllegalGlobalCall) {
//   std::string input = R"(
//         dialect:english;
//         func foo() int { return 1; }
//         let x int := foo(); // Error: Global initializers must be constant
//         expressions start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Global initializers must be constant");
// }
// // Error: Function call as an L-Value
// // Prevents the compiler from trying to assign a value to the *result* of a
// // function call.
// TEST_F(JITOutputTest, Error_Call_AssignmentToResult) {
//   std::string input = R"(
//         dialect:english;
//         func get_val() int { return 10; }
//         start() int {
//             get_val() := 20; // Error: Cannot assign to function result
//             return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Expression is not assignable");
// }
//
// //  Error: Type mismatch with unary negation inside call
// // Ensures the visitor correctly resolves the unary operator's type before
// // passing it to the function.
// TEST_F(JITOutputTest, Error_Call_NegatedTypeMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func process(val int) int { return val; }
//         start() int {
//             let s string := "test";
//             // Unary '-' is likely invalid for strings; visitor should
//             propagate this error let x int := process(-s); return 0;
//         }
//     )";
//   assertSwaOutput(input, "Error: Operator '-' not supported for type
//   'String'");
// }
//
// //  Error: Function identifier conflict with keyword
// // Prevents naming functions after internal keywords or reserved symbols.
// TEST_F(JITOutputTest, Error_Call_KeywordAsFunctionName) {
//   std::string input = R"(
//         dialect:english;
//         func if(a int) int { return a; } // Error: 'if' is a reserved keyword
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input,
//                   "Error: Illegal use of reserved keyword 'if' as
//                   identifier");
// }
//
// //  Error: Argument count with potential "default" value ambiguity
// // Tests if your parser/checker incorrectly assumes existence of default
// params. TEST_F(JITOutputTest, Error_Call_ExplicitArityViolation) {
//   std::string input = R"(
//         dialect:english;
//         func add(a int, b int) int { return a + b; }
//         start() int {
//             // Testing explicitly that we do NOT allow implied missing
//             arguments return add(10);
//         }
//     )";
//   assertSwaOutput(input, "Error: Argument count mismatch for 'add'");
// }
//
// //  Error: Call within a condition expression of an `if` block
// // Ensures that the boolean-only requirement for `if` conditions propagates
// // through the function's return type.
// TEST_F(JITOutputTest, Error_Call_InvalidReturnTypeInIf) {
//   std::string input = R"(
//         dialect:english;
//         func get_int() int { return 1; }
//         start() int {
//             // 'if' requires a boolean condition, but get_int() returns an
//             int if (get_int()) { return 0; } return 1;
//         }
//     )";
//   assertSwaOutput(input,
//                   "Error: Condition expression requires 'Bool', found
//                   'Int'");
// }
//
// //  Error: Recursive call exceeding scope definition
// // Validates that the return type of a recursive function is fixed by the
// // *first* declaration, and mismatches in the body are caught.
// TEST_F(JITOutputTest, Error_Call_RecursiveBodyMismatch) {
//   std::string input = R"(
//         dialect:english;
//         func recursive(n int) int {
//             // Even if 'recursive' is called here, it must match the return
//             type 'int' return recursive(n - 1) + "oops";
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Cannot perform '+' on Int and String");
// }
// //  Error: Argument-Dependent Resolution Failure
// // Tests that your lookup doesn't accidentally pick up a function from a
// child
// // scope when the function is called in the parent scope.
// TEST_F(JITOutputTest, Error_Call_ScopeLookupIsolation) {
//   std::string input = R"(
//         dialect:english;
//         start() int {
//             {
//                 func local_func() int { return 1; }
//             }
//             // Error: 'local_func' is not visible here
//             return local_func();
//         }
//     )";
//   assertSwaOutput(input, "Error: Function 'local_func' is undefined");
// }
// //  Error: Recursive Call with Mismatched Argument Count
// // Ensures that the recursive call is checked against the prototype, not the
// // call site.
// TEST_F(JITOutputTest, Error_Call_RecursiveArity) {
//   std::string input = R"(
//         dialect:english;
//         func recurse(a int) int {
//             return recurse(a, 10); // Error: expected 1 arg, got 2
//         }
//         start() int { return 0; }
//     )";
//   assertSwaOutput(input, "Error: Argument count mismatch for 'recurse'");
// }
