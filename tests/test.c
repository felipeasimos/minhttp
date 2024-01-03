#include "../minhttp.h"
#include "ctdd.h"
#include <string.h>

#define MAX_BUFFER_LEN 1096

mh_version version;
char* method;
uint8_t method_len;
char* path = NULL;
uint32_t path_len = 1096;
char* phrase = NULL;
uint32_t phrase_len = 1096;
uint32_t num_headers = 5;
mh_header headers[5] = {0};
uint16_t status = 0;

// examples taken from picohttpparser

char* simple_example = "GET / HTTP/1.0\r\n\r\n";
char* partial_example = "GET / HTTP/1.0\r\n\r";
char* test_headers_example = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nCookie: \r\n\r\n";
char* multibyte_example = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nUser-Agent: \343\201\262\343/1.0\r\n\r\n";
char* multiline_example = "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n  \tc\r\n\r\n";
char* multiline_success_example = "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n\r\n";
char* trailing_colon_example = "GET / HTTP/1.0\r\nfoo : ab\r\n\r\n";
char* trailing_value_example = "GET / HTTP/1.0\r\nfoo: a \t \r\n\r\n";
char* empty_name_example = "GET / HTTP/1.0\r\n:a\r\n\r\n";
char* bench_example = "GET /wp-content/uploads/2010/03/hello-kitty-darth-vader-pink.jpg HTTP/1.1\r\n" \
    "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n" \
    "Cookie: wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; "                                                          \
    "__utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; "                                                             \
    "__utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral\r\n\r\n";

char* nul_in_method_example = "G\0T / HTTP/1.0\r\n\r\n";
char* tab_in_method_example = "G\tT / HTTP/1.0\r\n\r\n";
char* invalid_method_example = ":GET / HTTP/1.0\r\n\r\n";
char* multiple_whitespace_example = "GET   /   HTTP/1.0\r\n\r\n";

char* response_1_1_example = "HTTP/1.1 200 OK\r\nHost: example.com\r\nCookie: \r\n\r\n";
char* response_1_0_example = "HTTP/1.0 200 OK\r\nfoo: \r\nfoo: b\r\n  \tc\r\n\r\n";
char* response_error_example = "HTTP/1.0 500 Internal Server Error\r\n\r\n";
char* response_incomplete_version_example = "HTTP/1. 200 OK\r\n\r\n";
char* response_wrong_version_example = "HTTP/1.2z 200 OK\r\n\r\n";
char* response_no_status_example = "HTTP/1.1  OK\r\n\r\n";
char* response_no_phrase_example = "HTTP/1.1 200\r\n\r\n";
char* response_garbage_1_example = "HTTP/1.1 200X\r\n\r\n";
char* response_garbage_2_example = "HTTP/1.1 200X \r\n\r\n";
char* response_garbage_3_example = "HTTP/1.1 200X OK\r\n\r\n";
char* response_multiple_whitespace_example = "HTTP/1.1   200   OK\r\n\r\n";

ctdd_test(parse_request_first_line_simple_test) {
  char* data = mh_parse_request_first_line(simple_example, simple_example + strlen(simple_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &simple_example[16], "return addr is wrong");
  ctdd_assert(strncmp(method, "GET", method_len) == 0, "method is wrong");
  ctdd_assert(strncmp(path, "/", path_len) == 0, "path is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
}

ctdd_test(parse_request_first_line_test_headers_test) {
  char* data = mh_parse_request_first_line(test_headers_example, test_headers_example + strlen(test_headers_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &test_headers_example[20], "return addr is wrong");
  ctdd_assert(strncmp(method, "GET", method_len) == 0, "method is wrong");
  ctdd_assert(strncmp(path, "/hoge", path_len) == 0, "path is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
}

ctdd_test(parse_request_first_line_nul_in_method_test) {
  char* data = mh_parse_request_first_line(nul_in_method_example, nul_in_method_example + strlen(nul_in_method_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_tab_in_method_test) {
  char* data = mh_parse_request_first_line(tab_in_method_example, tab_in_method_example + strlen(tab_in_method_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_invalid_method_test) {
  char* data = mh_parse_request_first_line(invalid_method_example, invalid_method_example + strlen(invalid_method_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data == NULL, "return addr is wrong");
}

ctdd_test(parse_request_first_line_multiple_whitespace_test) {
  char* data = mh_parse_request_first_line(multiple_whitespace_example, multiple_whitespace_example + strlen(multiple_whitespace_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data == multiple_whitespace_example + 20, "data is wrong");
  ctdd_assert(strncmp(method, "GET", method_len) == 0, "method is wrong");
  ctdd_assert(strncmp(path, "/", path_len) == 0, "path is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
}

ctdd_test(parse_request_first_line_bench_test) {
  char* data = mh_parse_request_first_line(bench_example, bench_example + strlen(bench_example), &method, &method_len, &path, &path_len, &version);
  ctdd_assert(data == bench_example + 75, "data is wrong");
  ctdd_assert(strncmp(method, "GET", method_len) == 0, "method is wrong");
  ctdd_assert(strncmp(path, "/wp-content/uploads/2010/03/hello-kitty-darth-vader-pink.jpg", path_len) == 0, "path is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
}

ctdd_test_suite(suite_parse_request_first_line) {
  ctdd_run_test(parse_request_first_line_simple_test);
  ctdd_run_test(parse_request_first_line_test_headers_test);
  ctdd_run_test(parse_request_first_line_nul_in_method_test);
  ctdd_run_test(parse_request_first_line_tab_in_method_test);
  ctdd_run_test(parse_request_first_line_invalid_method_test);
  ctdd_run_test(parse_request_first_line_multiple_whitespace_test);
  ctdd_run_test(parse_request_first_line_bench_test);
}

ctdd_test(parse_headers_simple_test) {
  char* data = mh_parse_headers(simple_example + strlen(simple_example) - 2, simple_example + strlen(simple_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == simple_example + strlen(simple_example), "data is wrong");
  ctdd_assert(num_headers == 0, "num_headers is wrong");
}

ctdd_test(parse_headers_partial_test) {
  char* data = mh_parse_headers(partial_example + strlen(partial_example) - 1, partial_example + strlen(partial_example), headers, &num_headers);
  ctdd_assert(data == NULL, "data is NULL");
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


ctdd_test(parse_headers_trailing_colon_example_test) {
  char* data = mh_parse_headers(trailing_colon_example + 16, trailing_colon_example + strlen(trailing_colon_example), headers, &num_headers);
  ctdd_assert(!data, "data is wrong");
  ctdd_assert(num_headers == 0, "num_headers is wrong");
}

ctdd_test(parse_headers_trailing_value_example_test) {
  char* data = mh_parse_headers(trailing_value_example + 16, trailing_value_example + strlen(trailing_value_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == trailing_value_example + strlen(trailing_value_example), "data is wrong");
  ctdd_assert(num_headers == 1, "num_headers is wrong");

  ctdd_assert(headers[0].header_key_len == strlen("foo"), "header[0] key len is wrong");
  ctdd_assert(strncmp(headers[0].header_key_begin, "foo", headers[0].header_key_len) == 0, "header[0] key is wrong");
  ctdd_assert(headers[0].header_value_len == strlen("a"), "header[0] value len is wrong");
  ctdd_assert(strncmp(headers[0].header_value_begin, "a", headers[0].header_value_len) == 0, "header[0] value is wrong");
}

ctdd_test(parse_headers_empty_name_example_test) {
  char* data = mh_parse_headers(empty_name_example + 16, empty_name_example + strlen(empty_name_example), headers, &num_headers);
  ctdd_assert(!data, "data is wrong");
  ctdd_assert(num_headers == 0, "num_headers is wrong");
}

ctdd_test(parse_headers_bench_test) {
  char* data = mh_parse_headers(bench_example + 75, bench_example + strlen(bench_example), headers, &num_headers);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == bench_example + strlen(bench_example), "data is wrong");
  ctdd_assert(num_headers == 2, "num_headers is wrong");

  ctdd_assert(headers[0].header_key_len == strlen("User-Agent"), "header[0] key len is wrong");
  ctdd_assert(strncmp(headers[0].header_key_begin, "User-Agent", headers[0].header_key_len) == 0, "header[0] key is wrong");
  ctdd_assert(headers[0].header_value_len == strlen("Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3"), "header[0] value len is wrong");
  ctdd_assert(strncmp(headers[0].header_value_begin, "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3", headers[0].header_value_len) == 0, "header[0] value is wrong");

  ctdd_assert(headers[1].header_key_len == strlen("Cookie"), "header[1] key len is wrong");
  ctdd_assert(strncmp(headers[1].header_key_begin, "Cookie", headers[1].header_key_len) == 0, "header[1] key is wrong");
  ctdd_assert(headers[1].header_value_len == strlen("wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; __utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; __utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral"), "header[1] value len is wrong");
  ctdd_assert(strncmp(headers[1].header_value_begin, "wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; __utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; __utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral", headers[1].header_value_len) == 0, "header[1] value is wrong");
}

ctdd_test_suite(suite_parse_headers) {
  ctdd_run_test(parse_headers_simple_test);
  ctdd_run_test(parse_headers_partial_test);
  ctdd_run_test(parse_headers_test_headers_example_test);
  ctdd_run_test(parse_headers_multibyte_example_test);
  ctdd_run_test(parse_headers_multiline_example_test);
  ctdd_run_test(parse_headers_multiline_success_example_test);
  ctdd_run_test(parse_headers_trailing_colon_example_test);
  ctdd_run_test(parse_headers_trailing_value_example_test);
  ctdd_run_test(parse_headers_empty_name_example_test);
  ctdd_run_test(parse_headers_bench_test);
}

ctdd_test(parse_response_first_line_1_1_test) {
  char* data = mh_parse_response_first_line(response_1_1_example, response_1_1_example + strlen(response_1_1_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &response_1_1_example[17], "return addr is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
  ctdd_assert(status == 200, "status code is wrong");
  ctdd_assert(phrase_len == 2, "phrase_len is wrong");
  ctdd_assert(strncmp(phrase, "OK", phrase_len) == 0, "phrase is wrong");
}

ctdd_test(parse_response_first_line_1_0_test) {
  char* data = mh_parse_response_first_line(response_1_0_example, response_1_0_example + strlen(response_1_0_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &response_1_0_example[17], "return addr is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
  ctdd_assert(status == 200, "status code is wrong");
  ctdd_assert(phrase_len == 2, "phrase_len is wrong");
  ctdd_assert(strncmp(phrase, "OK", phrase_len) == 0, "phrase is wrong");
}

ctdd_test(parse_response_first_line_error_test) {
  char* data = mh_parse_response_first_line(response_error_example, response_error_example + strlen(response_error_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &response_error_example[36], "return addr is wrong");
  ctdd_assert(version == HTTP_1, "version is wrong");
  ctdd_assert(status == 500, "status code is wrong");
  ctdd_assert(phrase_len == 21, "phrase_len is wrong");
  ctdd_assert(strncmp(phrase, "Internal Server Error", phrase_len) == 0, "phrase is wrong");
}

ctdd_test(parse_response_first_line_incomplete_version_test) {
  char* data = mh_parse_response_first_line(response_incomplete_version_example, response_incomplete_version_example + strlen(response_incomplete_version_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(data == NULL, "data is NULL");
}

ctdd_test(parse_response_first_line_wrong_version_test) {
  char* data = mh_parse_response_first_line(response_wrong_version_example, response_wrong_version_example + strlen(response_wrong_version_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(!data, "data is NULL");
}

ctdd_test(parse_response_first_line_no_status_test) {
  char* data = mh_parse_response_first_line(response_no_status_example, response_no_status_example + strlen(response_no_status_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(!data, "data is NULL");
}

ctdd_test(parse_response_first_line_no_phrase_test) {
  char* data = mh_parse_response_first_line(response_no_phrase_example, response_no_phrase_example + strlen(response_no_phrase_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &response_no_phrase_example[14], "return addr is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
  ctdd_assert(status == 200, "status code is wrong");
  ctdd_assert(phrase_len == 0, "phrase_len is wrong");
  ctdd_assert(strncmp(phrase, "", phrase_len) == 0, "phrase is wrong");
}

ctdd_test(parse_response_first_line_garbage_1_test) {
  char* data = mh_parse_response_first_line(response_garbage_1_example, response_garbage_1_example + strlen(response_garbage_1_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(!data, "data is NULL");
}

ctdd_test(parse_response_first_line_garbage_2_test) {
  char* data = mh_parse_response_first_line(response_garbage_2_example, response_garbage_2_example + strlen(response_garbage_2_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(!data, "data is NULL");
}

ctdd_test(parse_response_first_line_garbage_3_test) {
  char* data = mh_parse_response_first_line(response_garbage_3_example, response_garbage_3_example + strlen(response_garbage_3_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(!data, "data is NULL");
}

ctdd_test(parse_response_first_line_multiple_whitespace_test) {
  char* data = mh_parse_response_first_line(response_multiple_whitespace_example, response_multiple_whitespace_example + strlen(response_multiple_whitespace_example), &version, &status, &phrase, &phrase_len);
  ctdd_assert(data, "data is NULL");
  ctdd_assert(data == &response_multiple_whitespace_example[21], "return addr is wrong");
  ctdd_assert(version == HTTP_1_1, "version is wrong");
  ctdd_assert(status == 200, "status code is wrong");
  ctdd_assert(phrase_len == 2, "phrase_len is wrong");
  ctdd_assert(strncmp(phrase, "OK", phrase_len) == 0, "phrase is wrong");
}

// char* response_multiple_whitespace_example = "HTTP/1.1   200   OK\r\n\r\n";

ctdd_test_suite(suite_parse_response_first_line) {
  ctdd_run_test(parse_response_first_line_1_1_test);
  ctdd_run_test(parse_response_first_line_1_0_test);
  ctdd_run_test(parse_response_first_line_error_test);
  ctdd_run_test(parse_response_first_line_incomplete_version_test);
  ctdd_run_test(parse_response_first_line_wrong_version_test);
  ctdd_run_test(parse_response_first_line_no_status_test);
  ctdd_run_test(parse_response_first_line_no_phrase_test);
  ctdd_run_test(parse_response_first_line_garbage_1_test);
  ctdd_run_test(parse_response_first_line_garbage_2_test);
  ctdd_run_test(parse_response_first_line_garbage_3_test);
  ctdd_run_test(parse_response_first_line_multiple_whitespace_test);
}

void setup() {
  version = 0;
  method = NULL;
  method_len = 0;
  status = 0;
  path = NULL;
  path_len = MAX_BUFFER_LEN - 1;
  phrase = NULL;
  phrase_len = MAX_BUFFER_LEN - 1;
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
  ctdd_run_suite(suite_parse_response_first_line);
}
