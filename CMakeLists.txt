cmake_minimum_required(VERSION 3.14)
project(SDL2_snaek)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(SDL2_PATH "${CMAKE_SOURCE_DIR}/deps/SDL2-2.32.10/x86_64-w64-mingw32")
    set(IMG_PATH  "${CMAKE_SOURCE_DIR}/deps/SDL2_image-2.8.8/x86_64-w64-mingw32")
    set(MIX_PATH  "${CMAKE_SOURCE_DIR}/deps/SDL2_mixer-2.8.1/x86_64-w64-mingw32")
    set(TTF_PATH  "${CMAKE_SOURCE_DIR}/deps/SDL2_ttf-2.22.0/x86_64-w64-mingw32")

    set(CMAKE_PREFIX_PATH ${SDL2_PATH} ${IMG_PATH} ${MIX_PATH} ${TTF_PATH})
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(SDL2_snaek src/main.cpp)

if(WIN32)
    target_link_libraries(SDL2_snaek PRIVATE
        SDL2::SDL2main
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
    )
    
    target_link_options(SDL2_snaek PRIVATE -static)

    target_include_directories(SDL2_snaek PRIVATE
        "${SDL2_PATH}/include"
        "${IMG_PATH}/include"
        "${MIX_PATH}/include"
        "${TTF_PATH}/include"
    )

    file(GLOB DLL_FILES
        "${SDL2_PATH}/bin/*.dll"
        "${IMG_PATH}/bin/*.dll"
        "${MIX_PATH}/bin/*.dll"
        "${TTF_PATH}/bin/*.dll"
    )
    add_custom_command(TARGET SDL2_snaek POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL_FILES}
        $<TARGET_FILE_DIR:SDL2_snaek>
    )

else()
    target_link_libraries(SDL2_snaek PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
    )
endif()

add_custom_command(TARGET SDL2_snaek POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:SDL2_snaek>/assets
)