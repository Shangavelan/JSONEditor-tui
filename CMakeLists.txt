cmake_minimum_required(VERSION 3.10)
project(JSONEditorTUI C)

set(CMAKE_C_STANDARD 99)

add_executable(JSONEditorTUI 
    src/main.c
    src/tui.c
    src/cJSON.c
)

target_include_directories(JSONEditorTUI PRIVATE include)

# Find and link ncurses
find_package(Curses REQUIRED)
if(Curses_FOUND)
    target_include_directories(JSONEditorTUI PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(JSONEditorTUI PRIVATE ${CURSES_LIBRARIES})
endif()