cmake_minimum_required(VERSION 3.20)

project(tinyeditor)

include_directories(include)

set(SRC_FILES   
    "src/main.c")

add_executable(tinyeditor ${SRC_FILES})