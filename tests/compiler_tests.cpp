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
      "\x1B[1;31mtype check error\x1B[0m: my_func does not exist\n  --> "
      "swa_source:4:23\n   |\n 4 |       print(\"Result\", my_func());\n   |   "
      "                    "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\n\n";

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
      "0m Cannot '+' variable of type Int with a value of type "
      "String\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition "
      "\n  --> swa_source:1:20\n   |\n 1 | let x int := (10 + (20 + (30 + "
      "\"40\")));\n   |                    "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' "
      "variable of type Int with a value of type Dirty(TypeChecking "
      "failed)\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition "
      "\n  --> swa_source:1:14\n   |\n 1 | let x int := (10 + (20 + (30 + "
      "\"40\")));\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type Dirty(TypeChecking failed)\n\n";
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
      "like '&&' or '||' if you intended to perform a logical operation.\n\n";
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
      "operation.\n\n";
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
      "type Int\n\n";
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
      "type Dirty(TypeChecking failed)\n\n";
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
      "\x1B[1;31mtype check error\x1B[0m: Type mismatch in division \n  --> "
      "swa_source:1:19\n   |\n 1 | let x int := 10 + 20 * \"3\";\n   |         "
      "          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '*' variable of type Int with a value of "
      "type String\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in "
      "addition \n  --> swa_source:1:14\n   |\n 1 | let x int := 10 + 20 * "
      "\"3\";\n   |              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type Dirty(TypeChecking failed)\n\n";
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
      "0m Cannot '+' variable of type Int with a value of type "
      "String\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in  \n  --> "
      "swa_source:1:56\n   |\n 1 | let a int := 1; let b int := 2; let c int "
      ":= (a + (b * (10 / (a + \"str\"))));\n   |                              "
      "                          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '/' "
      "variable of type Int with a value of type Dirty(TypeChecking "
      "failed)\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in division "
      "\n  --> swa_source:1:51\n   |\n 1 | let a int := 1; let b int := 2; let "
      "c int := (a + (b * (10 / (a + \"str\"))));\n   |                        "
      "                           "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m Cannot '*' variable of type Int with a value of "
      "type Dirty(TypeChecking failed)\n\n\x1B[1;31mtype check error\x1B[0m: "
      "Type mismatch in addition \n  --> swa_source:1:46\n   |\n 1 | let a int "
      ":= 1; let b int := 2; let c int := (a + (b * (10 / (a + \"str\"))));\n  "
      " |                                              "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' "
      "variable of type Int with a value of type Dirty(TypeChecking "
      "failed)\n\n";
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
      "type Dirty(TypeChecking failed)\n\n";
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
      "String with a value of type Int\n\n";
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
      "String with a value of type Int\n\n";
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
      "\x1B[1;31mtype check error\x1B[0m: y does not exist\n  --> "
      "swa_source:8:26\n   |\n 8 |             let z int := y + 5;\n   |       "
      "                   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n\x1B[1;31mtype check "
      "error\x1B[0m: Type mismatch in addition \n  --> swa_source:8:26\n   |\n "
      "8 |             let z int := y + 5;\n   |                          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type "
      "Dirty(TypeChecking failed) with a value of type Int\n\n";
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
      "0m Cannot '-' variable of type Int with a value of type "
      "String\n\n\x1B[1;31mtype check error\x1B[0m: Type mismatch in addition "
      "\n  --> swa_source:6:26\n   |\n 6 |             let c int := (a * b) + "
      "(a - \"invalid\");\n   |                          "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type Dirty(TypeChecking failed)\n\n";
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
      "\x1B[1;31mtype check error\x1B[0m: b does not exist\n  --> "
      "swa_source:8:22\n   |\n 8 |     let c int := a + b;\n   |               "
      "       \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n\x1B[1;31mtype check "
      "error\x1B[0m: Type mismatch in addition \n  --> swa_source:8:18\n   |\n "
      "8 |     let c int := a + b;\n   |                  "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Cannot '+' variable of type Int "
      "with a value of type Dirty(TypeChecking failed)\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_Type_Mismatch) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1.5, 2.5];
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:4:29\n   |\n 4 |           let arr [2]int := [1.5, 2.5];\n   "
      "|                             "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected "
      "Array[2 * Int] but got Array[2 * Float]\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_SizeMismatch) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, 2, 3];
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:4:29\n   |\n 4 |           let arr [2]int := [1, 2, 3];\n   "
      "|                             "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected Array[2 * Int] but got "
      "Array[3 * Int]\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_TypeMismatch) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, "string"];
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Mixed datatatypes in array "
      "initialization\n  --> swa_source:4:33\n   |\n 4 |           let arr "
      "[2]int := [1, \"string\"];\n   |                                 "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\n   | \x1B[1;36mhelp\x1B[0m: Use only one "
      "datatype\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_OutOfBounds_Constant) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, 2];
          print(arr[5]);
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Out of bounds array access\n  --> "
      "swa_source:5:21\n   |\n 5 |           print(arr[5]);\n   |              "
      "       \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Index 5 is out of bounds\n  "
      " | \x1B[1;36mhelp\x1B[0m: Array contains 2 values and valid indexes go "
      "from 0 to 1\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_InvalidIndexType) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, 2];
          print(arr[1.5]);
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Array index must be of type int\n  "
      "--> swa_source:5:21\n   |\n 5 |           print(arr[1.5]);\n   |        "
      "             "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_InvalidIndexType_Expression) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, 2];
          let idx string := "index";
          print(arr[idx]);
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Array index must be of type int\n  "
      "--> swa_source:6:21\n   |\n 6 |           print(arr[idx]);\n   |        "
      "             "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_InvalidIndexType_Function) {
  std::string input = R"(
      dialect:english;
      func get_index() float { return 1.5; }
      start() int {
          let arr [2]int := [1, 2];
          print(arr[get_index()]);
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Array index must be of type int\n  "
      "--> swa_source:6:21\n   |\n 6 |           print(arr[get_index()]);\n   "
      "|                     "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_NegativeIndex) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, 2];
          print(arr[-1]);
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Array index must be positive\n  --> "
      "swa_source:5:21\n   |\n 5 |           print(arr[-1]);\n   |             "
      "        \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_OutOfBounds_Exact) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2]int := [1, 2];
          print(arr[2]);
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Out of bounds array access\n  --> "
      "swa_source:5:21\n   |\n 5 |           print(arr[2]);\n   |              "
      "       \x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Index 2 is out of bounds\n  "
      " | \x1B[1;36mhelp\x1B[0m: Array contains 2 values and valid indexes go "
      "from 0 to 1\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_Access_EmptyArray) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [0]int := [];
          return arr[0];
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Array initialization must have "
      "values\n  --> swa_source:4:29\n   |\n 4 |           let arr [0]int := "
      "[];\n   |                             "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n   | "
      "\x1B[1;36mhelp\x1B[0m: Add values to the array\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_NestedDimensionMismatch) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2][2]int := [[1], [3]];
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Incompatible type\n  --> "
      "swa_source:4:32\n   |\n 4 |           let arr [2][2]int := [[1], "
      "[3]];\n   |                                "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m Expected "
      "Array[2 * Array[2 * Int]] but got Array[2 * Array[1 * Int]]\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Error_Array_NestedDimensionMismatch_2) {
  std::string input = R"(
      dialect:english;
      start() int {
          let arr [2][2]int := [[1], [3, 4]];
          return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Mixed datatatypes in array "
      "initialization\n  --> swa_source:4:38\n   |\n 4 |           let arr "
      "[2][2]int := [[1], [3, 4]];\n   |                                      "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\n   | "
      "\x1B[1;36mhelp\x1B[0m: Use only one datatype\n\n";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Array_Nested_Access) {
  std::string input = R"(
      dialect:english;
      start() int {
        let matrix [2][2]int := [[1, 2], [3, 4]];
        print(matrix[1][0]);
        return 0;
      }
  )";
  std::string expected_diagnostic = "3";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Array_Access) {
  std::string input = R"(
      dialect:english;
      start() int {
        let arr [2]int := [10, 20];
        print(arr[1]);
        return 0;
      }
  )";
  std::string expected_diagnostic = "20";
  assertSwaOutput(input, expected_diagnostic);
}

TEST_F(JITOutputTest, Array_Access_With_Non_Array_Variable) {
  std::string input = R"(
      dialect:english;
      start() int {
        let arr int := 10;
        print(arr[1]);
        return 0;
      }
  )";
  std::string expected_diagnostic =
      "\x1B[1;31mtype check error\x1B[0m: Cannot index a non-array value\n  "
      "--> swa_source:5:15\n   |\n 5 |         print(arr[1]);\n   |            "
      "   "
      "\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B[0m\x1B[1;31m^\x1B["
      "0m the expression  has type 'Int', but an 'Array' is required\n   | "
      "\x1B[1;36mhelp\x1B[0m: Ensure the expression is declared as an array "
      "before attempting to index it\n\n";
  assertSwaOutput(input, expected_diagnostic);
}
