#include "../minhttp.h"
#include "ctdd.h"
#include <string.h>

#define MAX_PATH_LEN 1096
mh_version version;
mh_method method;
char path[1096] = {0};
unsigned int path_len = 1096;
unsigned int num_headers = 5;
mh_header headers[5] = {0};

// examples taken from picohttpparser
char* simple_example = "GET / HTTP/1.0\r\n\r\n";
char* partial_example = "GET / HTTP/1.0\r\n\r";
char* test_headers_example = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nCookie: \r\n\r\n";
char* multibyte_example = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nUser-Agent: \343\201\262\343/1.0\r\n\r\n";
char* multiline_example = "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n  \tc\r\n\r\n";
char* multiline_success_example = "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n\r\n";
char* trailing_example = "GET / HTTP/1.0\r\nfoo : ab\r\n\r\n";
char* nul_in_method_example = "G\0T / HTTP/1.0\r\n\r\n";
char* tab_in_method_example = "G\tT / HTTP/1.0\r\n\r\n";
char* invalid_method_example = ":GET / HTTP/1.0\r\n\r\n";

ctdd_test(parse_request_first_line_simple_test) {
  char* data = mh_parse_request_first_line(simple_example, simple_example + strlen(simple_example), &method, path, &path_len, &version);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &simple_example[16], "return addr is wrong");
  ctdd_assert(method == GET, "method is wrong");
  ctdd_assert(strcmp(path, "/") == 0, "path is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
}

ctdd_test(parse_request_first_line_test_headers_test) {
  char* data = mh_parse_request_first_line(test_headers_example, test_headers_example + strlen(test_headers_example), &method, path, &path_len, &version);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &test_headers_example[20], "return addr is wrong");
  ctdd_assert(method == GET, "method is wrong");
  ctdd_assert(strcmp(path, "/hoge") == 0, "path is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
}

ctdd_test(parse_request_first_line_nul_in_method_test) {
  char* data = mh_parse_request_first_line(nul_in_method_example, nul_in_method_example + strlen(nul_in_method_example), &method, path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_tab_in_method_test) {
  char* data = mh_parse_request_first_line(tab_in_method_example, tab_in_method_example + strlen(tab_in_method_example), &method, path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_invalid_method_test) {
  char* data = mh_parse_request_first_line(invalid_method_example, invalid_method_example + strlen(invalid_method_example), &method, path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");

}


ctdd_test_suite(suite_parse_request_first_line) {
  ctdd_run_test(parse_request_first_line_simple_test);
  ctdd_run_test(parse_request_first_line_test_headers_test);
  ctdd_run_test(parse_request_first_line_nul_in_method_test);
  ctdd_run_test(parse_request_first_line_tab_in_method_test);
  ctdd_run_test(parse_request_first_line_invalid_method_test);
}

ctdd_test(parse_headers_simple_test) {
  char* data = mh_parse_headers(simple_example + strlen(simple_example) - 2, simple_example + strlen(simple_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == simple_example + strlen(simple_example), "data is wrong");
  ctdd_assert(num_headers == 0, "num_headers is wrong");
}

ctdd_test(parse_headers_partial_test) {
  char* data = mh_parse_headers(partial_example + strlen(partial_example) - 1, partial_example + strlen(partial_example), headers, &num_headers);
  ctdd_assert(data == NULL, "data is wrong");
  ctdd_assert(num_headers == 0, "num_headers is wrong");
}

ctdd_test(parse_headers_test_headers_example_test) {
  char* data = mh_parse_headers(test_headers_example + 20, test_headers_example + strlen(test_headers_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == test_headers_example + strlen(test_headers_example), "data is wrong");
  ctdd_assert(num_headers == 2, "num_headers is wrong");

  ctdd_assert(headers[0].header_key_len == strlen("Host"), "header[0] key len is wrong");
  ctdd_assert(strncmp(headers[0].header_key_begin, "Host", headers[0].header_key_len) == 0, "header[0] key is wrong");
  ctdd_assert(headers[0].header_value_len == strlen("example.com"), "header[0] value len is wrong");
  ctdd_assert(strncmp(headers[0].header_value_begin, "example.com", headers[0].header_value_len) == 0, "header[0] value is wrong");

  ctdd_assert(headers[1].header_key_len == strlen("Cookie"), "header[1] key len is wrong");
  ctdd_assert(strncmp(headers[1].header_key_begin, "Cookie", headers[1].header_key_len) == 0, "header[1] key is wrong");
  ctdd_assert(headers[1].header_value_len == 0, "header[1] value len is wrong");
  ctdd_assert(strncmp(headers[1].header_value_begin, "", headers[1].header_value_len) == 0, "header[1] value is wrong");
}

ctdd_test(parse_headers_multibyte_example_test) {
  char* data = mh_parse_headers(multibyte_example + 20, multibyte_example + strlen(multibyte_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == multibyte_example + strlen(multibyte_example), "data is wrong");
  ctdd_assert(num_headers == 2, "num_headers is wrong");

  ctdd_assert(headers[0].header_key_len == strlen("Host"), "header[0] key len is wrong");
  ctdd_assert(strncmp(headers[0].header_key_begin, "Host", headers[0].header_key_len) == 0, "header[0] key is wrong");
  ctdd_assert(headers[0].header_value_len == strlen("example.com"), "header[0] value len is wrong");
  ctdd_assert(strncmp(headers[0].header_value_begin, "example.com", headers[0].header_value_len) == 0, "header[0] value is wrong");

  ctdd_assert(headers[1].header_key_len == strlen("User-Agent"), "header[1] key len is wrong");
  ctdd_assert(strncmp(headers[1].header_key_begin, "User-Agent", headers[1].header_key_len) == 0, "header[1] key is wrong");
  ctdd_assert(headers[1].header_value_len == strlen("\343\201\262\343/1.0"), "header[1] value len is wrong");
  ctdd_assert(strncmp(headers[1].header_value_begin, "\343\201\262\343/1.0", headers[1].header_value_len) == 0, "header[1] value is wrong");
}

ctdd_test(parse_headers_multiline_example_test) {
  char* data = mh_parse_headers(multiline_example + 16, multiline_example + strlen(multiline_example), headers, &num_headers);
  ctdd_assert(data == NULL, "data is NULL");
  ctdd_assert(num_headers == 0, "num_headers is wrong");
}

ctdd_test(parse_headers_multiline_success_example_test) {
  char* data = mh_parse_headers(multiline_success_example + 16, multiline_success_example + strlen(multiline_success_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == multiline_success_example + strlen(multiline_success_example), "data is wrong");
  ctdd_assert(num_headers == 2, "num_headers is wrong");

  ctdd_assert(headers[0].header_key_len == strlen("foo"), "header[0] key len is wrong");
  ctdd_assert(strncmp(headers[0].header_key_begin, "foo", headers[0].header_key_len) == 0, "header[0] key is wrong");
  ctdd_assert(headers[0].header_value_len == strlen(""), "header[0] value len is wrong");
  ctdd_assert(strncmp(headers[0].header_value_begin, "", headers[0].header_value_len) == 0, "header[0] value is wrong");

  ctdd_assert(headers[1].header_key_len == strlen("foo"), "header[1] key len is wrong");
  ctdd_assert(strncmp(headers[1].header_key_begin, "foo", headers[1].header_key_len) == 0, "header[1] key is wrong");
  ctdd_assert(headers[1].header_value_len == strlen("b"), "header[1] value len is wrong");
  ctdd_assert(strncmp(headers[1].header_value_begin, "b", headers[1].header_value_len) == 0, "header[1] value is wrong");
}

// char* multiline_example = "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n  \tc\r\n\r\n";
// char* trailing_example = "GET / HTTP/1.0\r\nfoo : ab\r\n\r\n";

ctdd_test_suite(suite_parse_headers) {
  ctdd_run_test(parse_headers_simple_test);
  ctdd_run_test(parse_headers_partial_test);
  ctdd_run_test(parse_headers_test_headers_example_test);
  ctdd_run_test(parse_headers_multibyte_example_test);
  ctdd_run_test(parse_headers_multiline_example_test);
  ctdd_run_test(parse_headers_multiline_success_example_test);
}

void setup() {
  version = 0;
  method = 0;
  memset(path, 0x00, MAX_PATH_LEN);
  path_len = MAX_PATH_LEN - 1;
  num_headers = 5;
  memset(headers, 0x00, num_headers * sizeof(mh_header));
}

void teardown() {
  return;
}

int main(int argc, char** argv) {
  int quiet = 0;
  if(argc > 1 && (!strcmp(argv[1], "--quiet") || !strcmp(argv[1], "-q") )) {
    quiet = 1;
  }
  ctdd_set_quiet(quiet);
  ctdd_configure(setup, teardown);
  ctdd_run_suite(suite_parse_request_first_line);
  ctdd_run_suite(suite_parse_headers);
}
