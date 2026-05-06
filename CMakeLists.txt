cmake_minimum_required(VERSION 3.10) # Lowered for better compatibility
project(SwaCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LLVM_DIR "/usr/lib/llvm-21/lib/cmake/llvm")
set(Clang_DIR "/usr/lib/llvm-21/lib/cmake/clang")

find_package(LLVM 21 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(include)

add_executable(swa
    src/lexer.cpp
    src/parser.cpp
    src/ast/symboltable.cpp
    src/codegen_visitor.cpp
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

add_executable(compiler_tests
  src/ast/symboltable.cpp
  src/lexer.cpp
  src/parser.cpp
  src/codegen_visitor.cpp
  src/compiler.cpp
  tests/lexer_tests.cpp
  tests/parser_tests.cpp
  tests/compiler_tests.cpp
  tests/main.cpp
)
target_link_libraries(compiler_tests
    GTest::gtest
    GTest::gtest_main
    pthread
    LLVM
)

install(TARGETS swa DESTINATION bin)
set(CPACK_PACKAGE_NAME "Swa")
set(CPACK_PACKAGE_VENDOR "Engagements")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_PACKAGE_CONTACT "pathe.sene@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "A multi dialect compiler")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_NAME "swacompiler")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "llvm (>= 21.0.0)")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop") # Creates a .dmg
elseif(UNIX)
  set(CPACK_GENERATOR "TGZ;DEB;RPM")  # Creates .tar.gz and .deb
endif()

include(CPack)
