cmake_minimum_required(VERSION 3.14)
project(SDL2_snaek)

# 1. Add SDL2 and Extensions (These are in app/jni/)
add_subdirectory(SDL2)
add_subdirectory(SDL2_image)
add_subdirectory(SDL2_mixer)
add_subdirectory(SDL2_ttf)

# 2. Compile your game code (Referencing src/main.cpp directly)
add_library(main SHARED
    src/main.cpp
)

# 3. Include headers
target_include_directories(main PRIVATE
    SDL2/include
    SDL2_image/include
    SDL2_mixer/include
    SDL2_ttf/include
)

# 4. Link libraries
target_link_libraries(main PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
    android
    log
    GLESv1_CM
    GLESv2
)
