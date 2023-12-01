#include "../minhttp.h"
#include "ctdd.h"
#include <string.h>

mh_version version;
mh_method method;
char path[1096] = {0};
unsigned int path_len = 1096;

// examples taken from picohttpparser
char* simple_example = "GET / HTTP/1.0\r\n\r\n";
char* partial_example = "GET / HTTP/1.0\r\n\r";
char* test_headers_example = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nCookie: \r\n\r\n";
char* multibyte_example = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nUser-Agent: \343\201\262\343/1.0\r\n\r\n";
char* multiline_example = "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n  \tc\r\n\r\n";
char* trailing_example = "GET / HTTP/1.0\r\nfoo : ab\r\n\r\n";

ctdd_test(parse_request_simple_test) {
  mh_parse_request_first_line(simple_example, &simple_example[16], &method, path, &path_len, &version);
  ctdd_assert(method == GET, "method is wrong");
  ctdd_assert(strcmp(path, "/") == 0, "path is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
}

ctdd_test_suite(test_parse_request) {
  ctdd_run_test(parse_request_simple_test);
}

int main() {
  ctdd_run_suite(test_parse_request);
}
