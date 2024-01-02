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

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
    SET(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/minhttp.dir)

    # add 'coverage' command to ninja/make
    add_custom_target(coverage
        COMMAND mkdir -p coverage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_custom_command(TARGET coverage
        COMMAND gcov -n -H ${CMAKE_SOURCE_DIR}/minhttp.c -o ${OBJECT_DIR}/minhttp.c.gcda* > CoverageSummary.tmp
        COMMAND cat CoverageSummary.tmp | grep -E '^Lines executed:' | head -n 1 | cut -d ':' -f 2 | cut -d ' ' -f 1
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
    )
endif() #CMAKE_BUILD_TYPE STREQUAL "Coverage"

