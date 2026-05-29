#!/bin/bash
# Usage: ./bundle-macos.sh <path_to_binary> <staging_folder>

BINARY=$1
STAGING=$2
FRAMEWORKS="$STAGING/Contents/Frameworks"

mkdir -p "$FRAMEWORKS"

# 1. Identify all LLVM libraries the binary depends on
LIBS=$(otool -L "$BINARY" | grep "libLLVM" | awk '{print $1}')

for LIB in $LIBS; do
  # 2. Copy the library into the bundle
  cp "$LIB" "$FRAMEWORKS/"

  # 3. Update the binary to point to the new location
  # This makes the reference relative to the binary's location
  install_name_tool -change "$LIB" "@executable_path/../Frameworks/$(basename "$LIB")" "$BINARY"
done

# 4. Optional: Recursively fix internal library references
# (In case the dylib itself depends on other dylibs)
for LIB in "$FRAMEWORKS"/*.dylib; do
  install_name_tool -id "@executable_path/../Frameworks/$(basename "$LIB")" "$LIB"
done
