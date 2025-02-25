cmake_minimum_required (VERSION 3.26)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project ("libcmdlinetest")

add_executable(libcmdlinetest)
target_sources(libcmdlinetest PRIVATE 
    "test.cpp" "optiontest.cpp" "switchtest.cpp" "argtest.cpp"
	"helptest.cpp" "parsertest.cpp"
)
add_dependencies(libcmdlinetest libcmdline)

set_property(TARGET libcmdlinetest PROPERTY CXX_STANDARD 17)

target_compile_definitions(libcmdlinetest
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
)

find_package(Catch2 3.6.0 REQUIRED)

target_include_directories(libcmdlinetest PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(libcmdlinetest PRIVATE libcmdline)
target_link_libraries(libcmdlinetest PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(libcmdlinetest)

