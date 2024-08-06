## MinHTTP

[![Linux](https://github.com/asimos-bot/minhttp/actions/workflows/test-linux.yml/badge.svg)](https://github.com/asimos-bot/minhttp/actions/workflows/test-linux.yml)
[![MacOS](https://github.com/asimos-bot/minhttp/actions/workflows/test-macos.yml/badge.svg)](https://github.com/asimos-bot/minhttp/actions/workflows/test-macos.yml)
[![Windows (MSVC)](https://github.com/asimos-bot/minhttp/actions/workflows/test-windows.yml/badge.svg)](https://github.com/asimos-bot/minhttp/actions/workflows/test-windows.yml)
[![No Dependencies](https://github.com/asimos-bot/minhttp/actions/workflows/no-includes.yml/badge.svg)](https://github.com/asimos-bot/minhttp/actions/workflows/no-includes.yml)
![Lines of Code Badge](https://img.shields.io/endpoint?url=https://gist.githubusercontent.com/felipeasimos/1062ce0f390bb2b6458d29f225cc08b5/raw/minhttp__heads_feat-coverage_coverage.json)

Minimal HTTP 1.0 and 1.1 parser and builder for requests and responses.

* no memory allocations
* no dependencies (only header used is `stdint.h`)

Inspired by picohttpparser. This project aims to have better performance and smaller size while having a builder feature.

### How to Use

There is two options:
* Build library and include it in your project
* Just copy `minhttp.c` and `minhttp.h` to the proper locations in your project.

### How to Build

1. `mkdir build/`
2. `cd build/`
3. `cmake .. && cmake --build .`

### How to Test

In `build/`: `./test` or `./test --quiet` (for no output on success).

The tests are based on the ones from [picohttpparser](https://github.com/h2o/picohttpparser/blob/master/test.c). Be aware that changes have been made due to difference between them:
* picohttpparser allows "keyless" values. This project doesn't 
    * compare their `"parse multiline"` test and this project's `"parse_headers_multiline_success_example_test"` and `"parse_headers_multiline_example_test"` for a better visualization.

### Roadmap

- [ ] parsers
    - [x] pass all tests for request first line from picohttpparser
    - [x] pass all tests for header parser from picohttpparser
    - [x] pass all tests for response first line parser from picohttpparser
    - [ ] parse only requested headers (`mh_parse_headers_set`)
- [ ] builder
    - [ ] header builder
    - [ ] response first line builder
    - [ ] response builder
    - [ ] request first line builder
- [ ] benchmarking and optimizations
    - [x] benchmark against other http parsers
        - [x] picohttpparser
    - [x] straight-forward parsing with no state machine
    - [x] add likely and unlikely in all appropriate jumps
    - [x] separate benchmark for header parsing and for first line parsing
    - [ ] enable NULL arguments for faster parsing
- [ ] misc
    - [x] add code coverage percentage
    - [ ] add documentation on how to use
