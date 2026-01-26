#!/bin/bash
set -e

BUILD_DIR="build/windows/temp"
OUTPUT_DIR="build/windows/output"
DEPS_DIR="build/windows/deps"

#rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR $OUTPUT_DIR $DEPS_DIR

if [ ! -d "$DEPS_DIR/SDL2-2.32.10/x86_64-w64-mingw32" ]; then
    echo "ERROR: SDL2 source not found in $DEPS_DIR"
    echo "Please download the MinGW development libraries (SDL2-devel-mingw.tar.gz)"
    exit 1
fi

echo "=== Generating Windows Resources ==="
magick assets/icon.png -define icon:auto-resize=256,128,64,48,32,16 "$BUILD_DIR/icon.ico"
echo '1 ICON "icon.ico"' > "$BUILD_DIR/resources.rc"

echo "=== Configuring Windows Build (MinGW) ==="
cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=./toolchain-mingw.cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=../../../$OUTPUT_DIR

echo "=== Building Project ==="
cmake --build $BUILD_DIR -j$(nproc)

echo "=== Done! ==="
echo "Executable located at: $OUTPUT_DIR"