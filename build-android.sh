#!/bin/bash

set -e

PROJECT_NAME=$(grep "project(" CMakeLists.txt | head -n 1 | sed -E 's/.*project\(([^)]+)\).*/\1/' | tr -d '[:space:]')

if [ -z "$PROJECT_NAME" ]; then
    echo "Error: Could not determine PROJECT_NAME from CMakeLists.txt"
    exit 1
fi

echo "Detected Project Name: $PROJECT_NAME"

echo "Generating Icons..."
magick ./assets/icon.png -resize 48x48   ./SDL_snaek_android/app/src/main/res/mipmap-mdpi/ic_launcher.png
magick ./assets/icon.png -resize 72x72   ./SDL_snaek_android/app/src/main/res/mipmap-hdpi/ic_launcher.png
magick ./assets/icon.png -resize 96x96   ./SDL_snaek_android/app/src/main/res/mipmap-xhdpi/ic_launcher.png
magick ./assets/icon.png -resize 144x144 ./SDL_snaek_android/app/src/main/res/mipmap-xxhdpi/ic_launcher.png
magick ./assets/icon.png -resize 192x192 ./SDL_snaek_android/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png

ANDROID_PROJECT_DIR="SDL_snaek_android"
OUTPUT_DIR="build/android"

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

echo "Building Android APK (Debug)..."
cd $ANDROID_PROJECT_DIR

./gradlew assembleDebug

# 3. Use the variable for the output file
echo "Copying APK to $OUTPUT_DIR..."
cp app/build/outputs/apk/debug/app-debug.apk ../$OUTPUT_DIR/${PROJECT_NAME}.apk

echo "Done! APK is located at: $OUTPUT_DIR/${PROJECT_NAME}.apk"