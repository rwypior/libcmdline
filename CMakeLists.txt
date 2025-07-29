cmake_minimum_required(VERSION 3.26)

set(CMAKE_DEBUG_POSTFIX "d")

project(libcmdline)

include("cmake/create_compile_commands.cmake")

add_library(libcmdline)
add_library(libcmdline::libcmdline ALIAS libcmdline)

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

# Install

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ConfigPackageLocation lib/cmake/libcmdline)

configure_package_config_file(
        cmake/libcmdlineConfig.cmake.in
	    "${CMAKE_CURRENT_BINARY_DIR}/libcmdline/libcmdlineConfig.cmake"
	    INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/libcmdline"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/libcmdline/libcmdlineConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY AnyNewerVersion
)

install(
        TARGETS libcmdline EXPORT libcmdlineTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
        DIRECTORY "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h"
)

install(EXPORT libcmdlineTargets
        FILE libcmdlineTargets.cmake
        NAMESPACE libcmdline::
        DESTINATION ${ConfigPackageLocation}
)

install(FILES 
            "${CMAKE_CURRENT_BINARY_DIR}/libcmdline/libcmdlineConfig.cmake" 
            "${CMAKE_CURRENT_BINARY_DIR}/libcmdline/libcmdlineConfigVersion.cmake"
        DESTINATION ${ConfigPackageLocation}
)

export(PACKAGE libcmdline)