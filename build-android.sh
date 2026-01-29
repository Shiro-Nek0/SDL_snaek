#!/bin/bash
set -e

PROJECT_NAME=$(grep "project(" CMakeLists.txt | head -n 1 | sed -E 's/.*project\(([^)]+)\).*/\1/' | tr -d '[:space:]')
if [ -z "$PROJECT_NAME" ]; then PROJECT_NAME="Game"; fi

ANDROID_PROJECT_DIR="android-project"
OUTPUT_DIR="build/android/output"
DEPS_DIR="build/android/deps"

echo "Detected Project Name: $PROJECT_NAME"
mkdir -p $OUTPUT_DIR $DEPS_DIR

if [ ! -d "$DEPS_DIR/SDL2-2.32.10" ]; then
    echo "ERROR: SDL2 source not found in $DEPS_DIR"
    echo "Please download the development libraries (SDL2-2.32.10.tar.gz)"
    exit 1
fi

echo "=== Generating Icons ==="
mkdir -p $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-mdpi
mkdir -p $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-hdpi
mkdir -p $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-xhdpi
mkdir -p $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-xxhdpi
mkdir -p $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-xxxhdpi

magick ./assets/icon.png -resize 48x48   $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-mdpi/ic_launcher.png
magick ./assets/icon.png -resize 72x72   $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-hdpi/ic_launcher.png
magick ./assets/icon.png -resize 96x96   $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-xhdpi/ic_launcher.png
magick ./assets/icon.png -resize 144x144 $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-xxhdpi/ic_launcher.png
magick ./assets/icon.png -resize 192x192 $ANDROID_PROJECT_DIR/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png

echo "=== Building Android APK ==="
cd $ANDROID_PROJECT_DIR
./gradlew assembleDebug

echo "=== Copying to Output ==="
cp app/build/outputs/apk/debug/app-debug.apk ../$OUTPUT_DIR/${PROJECT_NAME}.apk

echo "=== Done! ==="
echo "APK located at: $OUTPUT_DIR/${PROJECT_NAME}.apk"