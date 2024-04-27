cmake_minimum_required(VERSION 3.20)

project(tinyeditor)

include_directories(include)
include_directories(3rdparty/raygizmo/include)

add_subdirectory(3rdparty/raylib)
add_subdirectory(3rdparty/raylib-nuklear)

set(SRC_FILES   
    "src/main.c")

add_executable(tinyeditor ${SRC_FILES})

target_link_libraries(tinyeditor raylib raylib_nuklear)