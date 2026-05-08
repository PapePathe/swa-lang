# /usr/bin/bash

set -euxo pipefail

 LLVM_INSTALL_DIR=/usr/include/llvm/ 
 
 cmake -DLLVM_INSTALL_DIR=$LLVM_INSTALL_DIR \
       -DCMAKE_C_COMPILER=clang-21 \
       -DCMAKE_CXX_COMPILER=clang++ \
       -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
       -DLLVM_DIR=/usr/lib/llvm-21/lib/cmake/llvm \
       -G Ninja -B build .
 
 cd build
 cmake --build .
 
 ./compiler_tests
