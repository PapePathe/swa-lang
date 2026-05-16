# /usr/bin/bash

set -euxo pipefail

if [[ $# -eq 0 ]]; then
    echo "Error: No arguments provided."
    echo "Usage: $0 [argument]"
    exit 1
fi

 LLVM_INSTALL_DIR=/usr/include/llvm/

 cmake -DLLVM_INSTALL_DIR=$LLVM_INSTALL_DIR \
       -DCMAKE_C_COMPILER=clang-21 \
       -DCMAKE_CXX_COMPILER=clang++ \
       -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
       -DLLVM_DIR=/usr/lib/llvm-21/lib/cmake/llvm \
       -G Ninja -B build .

 cd build

if [[ "$1" == "test" ]]; then
 cmake --build . --target compiler_tests
 ./compiler_tests
elif [[ "$1" == "swa" ]]; then
 cmake --build . --target swa
./swa run
./swa build
./swa test
fi
