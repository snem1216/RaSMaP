cmake_minimum_required (VERSION 2.8.11)
project (rasmap)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable (rasmap rasmap.c)
target_link_libraries(rasmap ncurses)
