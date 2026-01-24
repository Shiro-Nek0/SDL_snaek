#!/bin/bash

set -e

magick ./assets/icon.png -resize 48x48 ./SDL_snaek_android/app/src/main/res/mipmap-mdpi/ic_launcher.png
magick ./assets/icon.png -resize 72x72 ./SDL_snaek_android/app/src/main/res/mipmap-hdpi/ic_launcher.png
magick ./assets/icon.png -resize 96x96 ./SDL_snaek_android/app/src/main/res/mipmap-xhdpi/ic_launcher.png
magick ./assets/icon.png -resize 14x144 ./SDL_snaek_android/app/src/main/res/mipmap-xxhdpi/ic_launcher.png
magick ./assets/icon.png -resize 192x192 ./SDL_snaek_android/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png

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