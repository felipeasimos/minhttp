## MinHTTP

Minimal HTTP 1.0 and 1.1 parser and builder for requests and responses

features:
* no memory allocations
* no dependencies

### Roadmap

- [ ] parsers
    - [x] request first line parser
    - [x] header parser
    - [x] add CMakeLists.txt
    - [x] add tests for request first line from picohttpparser
    - [ ] add tests for header parser  from picohttpparser
    - [ ] request parser
    - [ ] add tests from request parser from picohttpparser
    - [ ] response first line parser
    - [ ] response parser
    - [ ] parse only requested headers
        - [ ] get max key len automatically
- [ ] builder
    - [ ] header builder
    - [ ] response first line builder
    - [ ] response builder
    - [ ] request first line builder
    - [ ] request builder
- [ ] benchmarking and optimizations
    - [ ] enable NULL arguments for faster parsing
    - [ ] benchmark against other http parsers
        - [ ] picohttpparser
        - [ ] llhttp
    - [ ] add likely and unlikely in all appropriate jumps

