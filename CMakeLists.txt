cmake_minimum_required(VERSION 3.10) # Lowered for better compatibility
project(SwaCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(include)

add_executable(swa
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/swa.cpp 
)

llvm_map_components_to_libnames(llvm_libs
    core 
    support 
    native
)

target_link_libraries(swa LLVM)
find_package(GTest REQUIRED)

add_executable(lexer_tests
  src/lexer.cpp
  tests/lexer_tests.cpp
)
target_link_libraries(lexer_tests
    GTest::gtest
    GTest::gtest_main
    pthread
    LLVM
)

add_executable(parser_tests
  src/lexer.cpp
  src/parser.cpp
  tests/parser_tests.cpp
)
target_link_libraries(parser_tests
    GTest::gtest
    GTest::gtest_main
    pthread
    LLVM
)


add_executable(compiler_tests
  src/lexer.cpp
  src/parser.cpp
  src/compiler.cpp
  tests/compiler_tests.cpp
)
target_link_libraries(compiler_tests
    GTest::gtest
    GTest::gtest_main
    pthread
    LLVM
)
