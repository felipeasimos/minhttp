#include "../minhttp.h"
#include "ctdd.h"
#include <string.h>

#define MAX_PATH_LEN 1096
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
char* nul_in_method_example = "G\0T / HTTP/1.0\r\n\r\n";
char* tab_in_method_example = "G\tT / HTTP/1.0\r\n\r\n";
char* invalid_method_example = ":GET / HTTP/1.0\r\n\r\n";

ctdd_test(parse_request_first_line_simple_test) {
  char* data = mh_parse_request_first_line(simple_example, &simple_example[16], &method, path, &path_len, &version);
  ctdd_assert(data == &simple_example[16], "return addr is wrong");
  ctdd_assert(method == GET, "method is wrong");
  ctdd_assert(strcmp(path, "/") == 0, "path is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
}

ctdd_test(parse_request_first_line_test_headers_test) {
  char* data = mh_parse_request_first_line(test_headers_example, &test_headers_example[20], &method, path, &path_len, &version);
  ctdd_assert(data == &test_headers_example[20], "return addr is wrong");
  ctdd_assert(method == GET, "method is wrong");
  ctdd_assert(strcmp(path, "/hoge") == 0, "path is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
}

ctdd_test(parse_request_first_line_nul_in_method_test) {
  char* data = mh_parse_request_first_line(nul_in_method_example, &nul_in_method_example[16], &method, path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_tab_in_method_test) {
  char* data = mh_parse_request_first_line(tab_in_method_example, &nul_in_method_example[16], &method, path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_invalid_method_test) {
  char* data = mh_parse_request_first_line(invalid_method_example, &nul_in_method_example[17], &method, path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test_suite(test_parse_request_first_line) {
  ctdd_run_test(parse_request_first_line_simple_test);
  ctdd_run_test(parse_request_first_line_test_headers_test);
  ctdd_run_test(parse_request_first_line_nul_in_method_test);
  ctdd_run_test(parse_request_first_line_tab_in_method_test);
  ctdd_run_test(parse_request_first_line_invalid_method_test);
}

void setup() {
  version = 0;
  method = 0;
  memset(path, 0x00, MAX_PATH_LEN);
  path_len = MAX_PATH_LEN - 1;
}

void teardown() {
  return;
}

int main() {
  ctdd_configure(setup, teardown);
  ctdd_run_suite(test_parse_request_first_line);
}
