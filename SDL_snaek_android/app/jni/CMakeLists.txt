cmake_minimum_required(VERSION 3.14)
project(SDL2_snaek)

add_subdirectory(SDL2)
add_subdirectory(SDL2_image)
add_subdirectory(SDL2_mixer)
add_subdirectory(SDL2_ttf)

add_library(main SHARED
    ../../../src/main.cpp
)

target_include_directories(main PRIVATE
    SDL2/include
    SDL2_image/include
    SDL2_mixer/include
    SDL2_ttf/include
)

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
