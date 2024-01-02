#include "../minhttp.h"
#include "picohttpparser/picohttpparser.h"
#include <stdio.h>
#include <string.h>

#if defined(__has_include)
#if __has_include(<sys/time.h>)
  #include <sys/time.h>
  static unsigned long get_microseconds() {
    static struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000000 + time.tv_usec;
  }
#elif __has_include(<windows.h>)
  #include <windows.h>
  static unsigned long get_microseconds() {
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    double frequency = ((double)li.QuadPart)/1000000.0;
    QueryPerformanceCounter(&li);
    return (unsigned long)(li.QuadPart/frequency);
  }
#endif
#endif

#define REPETITIONS 1000000
#define MAX_BUFFER_LEN 1096

char* requests[] = { \
 // "GET / HTTP/1.0\r\n\r\n", // char* simple_example
 // "GET /hoge HTTP/1.1\r\nHost: example.com\r\nCookie: \r\n\r\n", // char* test_headers_example
 // "GET /hoge HTTP/1.1\r\nHost: example.com\r\nUser-Agent: \343\201\262\343/1.0\r\n\r\n", // char* multibyte_example
 // "GET / HTTP/1.0\r\nfoo: \r\nfoo: b\r\n\r\n", // char* multiline_success_example
 // "GET / HTTP/1.0\r\nfoo: a \t \r\n\r\n", // char* trailing_value_example
 // "GET   /   HTTP/1.0\r\n\r\n", // char* multiple_whitespace_example
 "GET /wp-content/uploads/2010/03/hello-kitty-darth-vader-pink.jpg HTTP/1.1\r\n"                                                \
    "Host: www.kittyhell.com\r\n"                                                                                                  \
    "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 "             \
    "Pathtraq/0.9\r\n"                                                                                                             \
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"                                                  \
    "Accept-Language: ja,en-us;q=0.7,en;q=0.3\r\n"                                                                                 \
    "Accept-Encoding: gzip,deflate\r\n"                                                                                            \
    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"                                                                            \
    "Keep-Alive: 115\r\n"                                                                                                          \
    "Connection: keep-alive\r\n"                                                                                                   \
    "Cookie: wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; "                                                          \
    "__utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; "                                                             \
    "__utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral\r\n"             \
    "\r\n"
};

char* responses[] = {
 "HTTP/1.1 200 OK\r\nHost: example.com\r\nCookie: \r\n\r\n", // char* response_1_1_example
 "HTTP/1.0 200 OK\r\nfoo: \r\nfoo: b\r\n  \tc\r\n\r\n", // char* response_1_0_example
 "HTTP/1.0 500 Internal Server Error\r\n\r\n", // char* response_error_example
 "HTTP/1. 200 OK\r\n\r\n", // char* response_incomplete_version_example
 "HTTP/1.1 200\r\n\r\n", // char* response_no_phrase_example
 "HTTP/1.1   200   OK\r\n\r\n", // char* response_multiple_whitespace_example
};

unsigned long num_responses = sizeof(responses)/sizeof(responses[0]);
unsigned long num_requests = sizeof(requests)/sizeof(requests[0]);

#define MINHTTP_BENCHMARK(func_call)\
    mh_version version;\
    mh_method method;\
    char path[MAX_BUFFER_LEN] = {0};\
    uint32_t path_len = MAX_BUFFER_LEN;\
    uint32_t num_headers = 10;\
    mh_header headers[10] = {0};\
  \
    unsigned long sum = 0;\
    for(unsigned long i = 0; i < REPETITIONS; i++) {\
        for(unsigned long j = 0; j < num_requests; j++) {\
          char* request = requests[j];\
          char* request_end = request + strlen(request);\
          version = 0;\
          method = 0;\
          memset(path, 0x00, MAX_BUFFER_LEN);\
          path_len = MAX_BUFFER_LEN - 1;\
          num_headers = 5;\
          memset(headers, 0x00, num_headers * sizeof(mh_header));\
          unsigned long start = get_microseconds();\
          func_call\
          sum += get_microseconds() - start;\
        }\
    }\
    return sum;\


unsigned long minhttp_total_benchmark() {
  MINHTTP_BENCHMARK(
      request = mh_parse_request_first_line(request, request_end, &method, path, &path_len, &version);\
      request = mh_parse_headers(request, request_end, headers, &num_headers);\
  );
}

unsigned long minhttp_header_benchmark() {
  MINHTTP_BENCHMARK(
      request = mh_parse_headers(request + 75, request_end, headers, &num_headers);\
  );
}

#define PICOHTTPPARSER_BENCHMARK(func_call)\
    const char *method;\
    size_t method_len;\
    const char *path;\
    size_t path_len;\
    int minor_version;\
    struct phr_header headers[32];\
    size_t num_headers;\
\
    unsigned long sum = 0;\
    for(unsigned long i = 0; i < REPETITIONS; i++) {\
        for(unsigned long j = 0; j < num_requests; j++) {\
          char* request = requests[j];\
          int request_size = strlen(request);\
          unsigned long start = get_microseconds();\
          func_call\
          sum += get_microseconds() - start;\
        }\
    }\
    return sum;


unsigned long picohttpparser_total_benchmark() {
    PICOHTTPPARSER_BENCHMARK(
        phr_parse_request(request, request_size, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, 0);
    );
}

unsigned long picohttpparser_header_benchmark() {
    PICOHTTPPARSER_BENCHMARK(
        phr_parse_headers(request + 75, request_size - 75, headers, &num_headers, 0);\
    );
}


unsigned long per_second(unsigned long elapsed_microsecs, unsigned long total_bench_objs) {
  return (1000000 * total_bench_objs) / elapsed_microsecs;
}

int main() {

  // total
  unsigned long minhttp_microsecs = minhttp_total_benchmark();
  unsigned long picohttpparser_microsecs = picohttpparser_total_benchmark();
  double performance = 1 - (double)picohttpparser_microsecs/(double)minhttp_microsecs;
  if(performance < 0) performance = 1 -performance;
  printf("total:\n");
  printf("\tminhttp performance compared to picotthpparser: %.2f%%\n", 100 * performance);

  // header
  minhttp_microsecs = minhttp_header_benchmark();
  picohttpparser_microsecs = picohttpparser_header_benchmark();
  performance = 1 - (double)picohttpparser_microsecs/(double)minhttp_microsecs;
  if(performance < 0) performance = 1 -performance;
  printf("headers:\n");
  printf("\tminhttp performance compared to picotthpparser: %.2f%%\n", 100 * performance);

  return 0;
}
