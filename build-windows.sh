#!/bin/bash

set -e

OUTPUT_DIR="build/windows/output"

cmake -S . -B $OUTPUT_DIR -DCMAKE_TOOLCHAIN_FILE=./toolchain-mingw.cmake

cmake --build $OUTPUT_DIR -j$(nproc)