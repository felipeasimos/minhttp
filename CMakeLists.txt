cmake_minimum_required(VERSION 3.27)
project(minhttp VERSION 0.1 LANGUAGES C)

add_library(minhttp STATIC minhttp.c)
add_library(picohttpparser STATIC tests/picohttpparser/picohttpparser.c)

add_executable(test tests/test.c)
add_executable(benchmark tests/benchmark.c)

target_link_libraries(test PUBLIC minhttp)
target_include_directories(test PUBLIC tests/)

target_link_libraries(benchmark PUBLIC picohttpparser minhttp)
target_include_directories(benchmark PUBLIC tests/picohttpparser)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
