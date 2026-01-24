#!/bin/bash

set -e

ANDROID_PROJECT_DIR="SDL_snaek_android"
OUTPUT_DIR="build-android"

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

echo "Building Android APK (Debug)..."
cd $ANDROID_PROJECT_DIR

./gradlew assembleDebug

echo "Copying APK to $OUTPUT_DIR..."
cp app/build/outputs/apk/debug/app-debug.apk ../$OUTPUT_DIR/SDL_snaek.apk

echo "Done! APK is located at: $OUTPUT_DIR/SDL_snaek.apk"