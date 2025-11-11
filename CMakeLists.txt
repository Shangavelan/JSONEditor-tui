cmake_minimum_required(VERSION 3.10)
project(JSONEditorTUI C)

set(CMAKE_C_STANDARD 99)

add_executable(JSONEditorTUI 
    src/main.c
    src/hello.c
)

target_include_directories(JSONEditorTUI PRIVATE include)
