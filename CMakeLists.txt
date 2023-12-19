cmake_minimum_required(VERSION 3.27)
project(minhttp VERSION 0.1 LANGUAGES C)

add_library(minhttp STATIC minhttp.c)

add_executable(test tests/test.c)

target_link_libraries(test PUBLIC minhttp)
target_include_directories(test PUBLIC tests/)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
