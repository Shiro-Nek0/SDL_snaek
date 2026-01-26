#!/bin/bash

set -e

BUILD_DIR="build/linux/temp"
OUTPUT_DIR="build/linux/output"

#rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR $OUTPUT_DIR

echo "=== Configuring Linux Build ==="
cmake -S . -B $BUILD_DIR -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$(pwd)/$OUTPUT_DIR"

echo "=== Building Project ==="
cmake --build $BUILD_DIR -j$(nproc)

echo "=== Done! ==="
echo "Executable is located at: $OUTPUT_DIR"
echo "Run it with: ./$OUTPUT_DIR/SDL2_snaek"