#!/bin/bash

set -e

OUTPUT_DIR="build/linux/output"

cmake -S . -B $OUTPUT_DIR

cmake --build $OUTPUT_DIR -j$(nproc)