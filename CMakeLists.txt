cmake_minimum_required(VERSION 3.14)

project(SDL2_snaek)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IMGUI_VER "1.92.5")
set(SDL2_VER "2.32.10")
set(IMG_VER "2.8.8")
set(MIX_VER "2.8.1")
set(TTF_VER "2.24.0")

set(SHARED_DEPS_ROOT "${CMAKE_SOURCE_DIR}/build/sharedDeps")

if(ANDROID)
    set(DEPS_ROOT "${CMAKE_SOURCE_DIR}/build/android/deps")
elseif(WIN32)
    set(DEPS_ROOT "${CMAKE_SOURCE_DIR}/build/windows/deps")
elseif(UNIX AND NOT APPLE)
    set(DEPS_ROOT "${CMAKE_SOURCE_DIR}/build/linux/deps")
endif()

set(IMGUI_PATH "${SHARED_DEPS_ROOT}/imgui-${IMGUI_VER}")

add_library(imgui_lib STATIC
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
)

if(ANDROID)
    add_compile_definitions(ASSET_PATH="")

    if(NOT EXISTS "${DEPS_ROOT}/SDL2-${SDL2_VER}")
        message(FATAL_ERROR "Android SDL2 deps missing at: ${DEPS_ROOT}/SDL2-${SDL2_VER}")
    endif()

    add_subdirectory(${DEPS_ROOT}/SDL2-${SDL2_VER} SDL2_build)
    add_subdirectory(${DEPS_ROOT}/SDL2_image-${IMG_VER} SDL2_image_build)
    add_subdirectory(${DEPS_ROOT}/SDL2_mixer-${MIX_VER} S1DL2_mixer_build)
    add_subdirectory(${DEPS_ROOT}/SDL2_ttf-${TTF_VER} SDL2_ttf_build)

    add_library(main SHARED src/main.cpp)

    target_include_directories(main PRIVATE
        ${DEPS_ROOT}/SDL2-${SDL2_VER}/include
        ${DEPS_ROOT}/SDL2_image-${IMG_VER}/include
        ${DEPS_ROOT}/SDL2_mixer-${MIX_VER}/include
        ${DEPS_ROOT}/SDL2_ttf-${TTF_VER}/include
    )

    target_link_libraries(imgui_lib PUBLIC SDL2::SDL2)
    target_link_libraries(main PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
        imgui_lib
        android log GLESv1_CM GLESv2
    )

else()
    add_compile_definitions(ASSET_PATH="assets/")

    add_executable(${PROJECT_NAME} src/main.cpp)

    if(WIN32)
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/resources.rc)

        set(SDL2_PATH "${DEPS_ROOT}/SDL2-${SDL2_VER}/x86_64-w64-mingw32")
        set(IMG_PATH "${DEPS_ROOT}/SDL2_image-${IMG_VER}/x86_64-w64-mingw32")
        set(MIX_PATH "${DEPS_ROOT}/SDL2_mixer-${MIX_VER}/x86_64-w64-mingw32")
        set(TTF_PATH "${DEPS_ROOT}/SDL2_ttf-${TTF_VER}/x86_64-w64-mingw32")

        list(APPEND CMAKE_PREFIX_PATH "${SDL2_PATH}" "${IMG_PATH}" "${MIX_PATH}" "${TTF_PATH}")

        target_link_options(${PROJECT_NAME} PRIVATE -static)
        target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)

        file(GLOB DLL_FILES
            "${SDL2_PATH}/bin/*.dll"
            "${IMG_PATH}/bin/*.dll"
            "${MIX_PATH}/bin/*.dll"
            "${TTF_PATH}/bin/*.dll"
        )
        
        if(DLL_FILES)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL_FILES}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        else()
            message(WARNING "No DLLs found in ${SDL2_PATH}/bin! The .exe might not run.")
        endif()
    endif()

    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(SDL2_ttf REQUIRED)

    target_link_libraries(imgui_lib PUBLIC SDL2::SDL2)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
        SDL2_ttf::SDL2_ttf
        imgui_lib
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
endif()