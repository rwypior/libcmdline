cmake_minimum_required(VERSION 3.26)

project(libcmdline)

include("cmake/create_compile_commands.cmake")

add_library(libcmdline)

target_include_directories(libcmdline PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_subdirectory("include/libcmdline")
add_subdirectory("src")

add_subdirectory("example")

if(${BUILD_TESTING})
    message("Building unit tests")

    include(CTest)
    add_subdirectory ("test")
endif()

