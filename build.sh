# /usr/bin/bash

set -euxo pipefail

 LLVM_INSTALL_DIR=/usr/include/llvm/ 
 
 cmake -DLLVM_INSTALL_DIR=$LLVM_INSTALL_DIR \
       -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
       -G Ninja -B build .
 
 cd build
 cmake --build .
 
 ./lexer_tests
 ./parser_tests
