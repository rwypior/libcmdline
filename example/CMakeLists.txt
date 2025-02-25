cmake_minimum_required(VERSION 3.26)

project(cmdlineexample)
add_executable(cmdlineexample main.cpp)

target_link_libraries(cmdlineexample libcmdline)
