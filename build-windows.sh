#!/bin/bash
set -e

BUILD_DIR="build/windows/temp"
OUTPUT_DIR="build/windows/output"
DEPS_DIR="build/windows/deps"

rm -rf $OUTPUT_DIR $BUILD_DIR
mkdir -p $BUILD_DIR
mkdir -p $OUTPUT_DIR

if [ ! -d "$DEPS_DIR/SDL2-2.32.10/x86_64-w64-mingw32" ]; then
    echo "============================================================"
    echo " ERROR: Dependencies missing in $DEPS_DIR"
    echo "============================================================"
    echo "Please download the MinGW development libraries (SDL2-devel-mingw.tar.gz)"
fi

echo "=== Configuring Windows Build (MinGW) ==="
cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=./toolchain-mingw.cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../../../$OUTPUT_DIR

echo "=== Building Project ==="
cmake --build $BUILD_DIR -j$(nproc)

echo "=== Done! ==="
echo "Executable located at: $OUTPUT_DIR"