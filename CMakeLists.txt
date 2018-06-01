cmake_minimum_required(VERSION 3.7)
project(CURSES)

set(CMAKE_C_STANDARD 90)
set(DEV_PATH "../dev")

link_directories("${DEV_PATH}/lib")
include_directories("${DEV_PATH}/include")

set(SOURCE_FILES CURSES.c)
add_executable(CURSES ${SOURCE_FILES})
target_link_libraries(CURSES Data_Structures.so webutils.so)