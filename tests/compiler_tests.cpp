#include <compiler/compiler.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>

class JITOutputTest : public ::testing::Test {
protected:
  std::string runAndCapture(std::function<void()> func) {
    testing::internal::CaptureStdout();

    func();

    return testing::internal::GetCapturedStdout();
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
        print("10 = %d", 10); 
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
        print("10 - 3 - 2 = %d", 10 - 3 - 2);
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
        print("16 / 4 / 2 = %d", 16 / 4 / 2);
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
        print("5 + 3 * 2 - 8 / 4 = %d", 5 + 3 * 2 - 8 / 4);
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
        print("7 / 2 = %d", 7/2);
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
        print("12 * 3 / 4 = %d", 12 * 3 / 4);
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
        print("5 - 3 + 2 = %d", 5 - 3 + 2);
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
        print("((2+3)*(4-1))/(5-2) = %d", ((2 + 3) * (4 - 1)) / (5 - 2));
      }
    )";
    SwaCompiler swa;
    swa.Run(program);
  });

  ASSERT_EQ(output, "((2+3)*(4-1))/(5-2) = 5");
}
