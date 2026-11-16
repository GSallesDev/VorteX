cmake_minimum_required(VERSION 3.20)
project(vortex VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        5.0
    GIT_SHALLOW    TRUE
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib raygui)

add_library(raygui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/raygui.c
)

target_include_directories(raygui PUBLIC
    ${raygui_SOURCE_DIR}/src
)

target_link_libraries(raygui PUBLIC raylib)

target_compile_options(raygui PRIVATE -w)

set(SOURCES
    src/main.c
    src/app.c
    src/assets.c
    src/scene.c
    src/scene_menu.c
    src/scene_game.c
)

add_executable(vortex ${SOURCES})

target_include_directories(vortex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vortex PRIVATE raylib raygui)

target_compile_options(vortex PRIVATE
    -Wall -Wextra -Wpedantic -ggdb
)

add_custom_command(TARGET vortex POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:vortex>/res
)


