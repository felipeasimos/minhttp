## MinHTTP

Minimal HTTP 1.0 and 1.1 parser and builder for requests and responses

features:
* no memory allocations
* no dependencies

### Roadmap

- [ ] parsers
    - [x] request first line parser
    - [x] header parser
    - [ ] add tests for header parser and request first line parser from picohttpparser
    - [ ] add CMakeLists.txt
    - [ ] request parser
    - [ ] response first line parser
    - [ ] response parser
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

