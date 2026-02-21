cmake_minimum_required(VERSION 3.20)
project(filesystem_utils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(filesystem_utils INTERFACE)
add_library(filesystem_utils::filesystem_utils ALIAS filesystem_utils)

target_include_directories(filesystem_utils INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(filesystem_utils INTERFACE /W4 /permissive-)
else()
  target_compile_options(filesystem_utils INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(filesystem_utils_basic_test tests/test_basic.cpp)
target_link_libraries(filesystem_utils_basic_test PRIVATE filesystem_utils::filesystem_utils)
add_test(NAME filesystem_utils.basic COMMAND filesystem_utils_basic_test)
