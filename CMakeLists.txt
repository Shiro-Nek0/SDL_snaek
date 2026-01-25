cmake_minimum_required(VERSION 3.14)

project(SDL2_snaek)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SDL2_VER "2.32.10")
set(IMG_VER  "2.8.8")
set(MIX_VER  "2.8.1")
set(TTF_VER  "2.24.0")

if(ANDROID)
    add_compile_definitions(ASSET_PATH="")

    set(ANDROID_DEPS_ROOT "${CMAKE_SOURCE_DIR}/build/android/deps")

    add_subdirectory(${ANDROID_DEPS_ROOT}/SDL2-${SDL2_VER}       SDL2_build)
    add_subdirectory(${ANDROID_DEPS_ROOT}/SDL2_image-${IMG_VER}  SDL2_image_build)
    add_subdirectory(${ANDROID_DEPS_ROOT}/SDL2_mixer-${MIX_VER}  SDL2_mixer_build)
    add_subdirectory(${ANDROID_DEPS_ROOT}/SDL2_ttf-${TTF_VER}    SDL2_ttf_build)

    add_library(main SHARED src/main.cpp)

    target_include_directories(main PRIVATE
        ${ANDROID_DEPS_ROOT}/SDL2-${SDL2_VER}/include
        ${ANDROID_DEPS_ROOT}/SDL2_image-${IMG_VER}/include
        ${ANDROID_DEPS_ROOT}/SDL2_mixer-${MIX_VER}/include
        ${ANDROID_DEPS_ROOT}/SDL2_ttf-${TTF_VER}/include
    )

    target_link_libraries(main PRIVATE 
        SDL2::SDL2 
        SDL2_image::SDL2_image 
        SDL2_mixer::SDL2_mixer 
        SDL2_ttf::SDL2_ttf 
        android log GLESv1_CM GLESv2
    )

else()
    add_compile_definitions(ASSET_PATH="assets/")
    
    add_executable(${PROJECT_NAME} src/main.cpp)

    if(WIN32)
        set(DEPS_ROOT "${CMAKE_SOURCE_DIR}/build/windows/deps")

        set(SDL2_PATH "${DEPS_ROOT}/SDL2-${SDL2_VER}/x86_64-w64-mingw32")
        set(IMG_PATH  "${DEPS_ROOT}/SDL2_image-${IMG_VER}/x86_64-w64-mingw32")
        set(MIX_PATH  "${DEPS_ROOT}/SDL2_mixer-${MIX_VER}/x86_64-w64-mingw32")
        set(TTF_PATH  "${DEPS_ROOT}/SDL2_ttf-${TTF_VER}/x86_64-w64-mingw32")

        list(APPEND CMAKE_PREFIX_PATH "${SDL2_PATH}" "${IMG_PATH}" "${MIX_PATH}" "${TTF_PATH}")

        target_link_options(${PROJECT_NAME} PRIVATE -static)
        target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)

        file(GLOB DLL_FILES
            "${SDL2_PATH}/bin/*.dll"
            "${IMG_PATH}/bin/*.dll"
            "${MIX_PATH}/bin/*.dll"
            "${TTF_PATH}/bin/*.dll"
        )
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DLL_FILES}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()

    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(SDL2_ttf REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        SDL2::SDL2 
        SDL2_image::SDL2_image 
        SDL2_mixer::SDL2_mixer 
        SDL2_ttf::SDL2_ttf
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
endif()