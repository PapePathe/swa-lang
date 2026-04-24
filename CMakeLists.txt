cmake_minimum_required(VERSION 3.10) # Lowered for better compatibility
project(SwaCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Setup LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 2. Include our headers
include_directories(include)

# 3. Add executable
add_executable(swa
    src/lexer.cpp 
    src/swa.cpp 
)

# 4. Link LLVM Libraries (CRITICAL: Added 'core')
# 'core' contains Module, Function, IRBuilder, etc.
llvm_map_components_to_libnames(llvm_libs 
    core 
    support 
    native
)

target_link_libraries(swa LLVM)

# 5. Testing
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
