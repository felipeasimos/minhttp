#include "minhttp.h"
#include <stdint.h>
#include <stddef.h>

#define MIN(a, b) (a < b ? a : b)
#if __GNUC__ >= 3
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define CHECK_EOF() if(data == data_end) return NULL;

#define EXPECT_NO_CHECK(x) CHECK_EOF(); if(unlikely(*data != x)) return NULL; data++;

#define EXPECT(x) CHECK_EOF(); EXPECT_NO_CHECK(x); if(data_end <= data) return NULL;

#define UNTIL_NOT(x) for(; data != data_end && *data == x; data++);


#define EXPECT_NEWLINE() do {\
  if(data_end == data) return NULL;\
  switch(*data) {\
    case '\r': {\
      if(data_end - data < 2) return NULL;\
      if(*(data+1) == '\n') return data + 2;\
      return NULL;\
    }\
    case '\n': {\
      return data + 1;\
    }\
    default: {\
      return NULL;\
    }\
  }\
} while(0);

char* _mh_parse_method(char* data, unsigned int data_len, mh_method* method) {
  if(data_len < 3) return NULL;
  switch(*data) {
    // GET
    case 'G': {
      if(*(data+1) != 'E' || *(data+2) != 'T') return NULL;
      *method = GET;
      return data + 3;
    }
    // PUT
    // POST
    // PATCH
    case 'P': {
      switch(*(data+1)) {
        case 'U': {
          if(*(data+2) != 'T') return NULL;
          *method = PUT;
          return data + 3;
        }
        case 'O': {
          if(data_len == 3) return NULL;
          if(*(data+2) != 'S' || *(data+3) != 'T') return NULL;
          *method = POST;
          return data + 4;
        }
        case 'A': {
          if(data_len == 4) return NULL;
          if(*(data+2) != 'T' || *(data+3) != 'C' || *(data+4) != 'H') return NULL;
          * method = PATCH;
          return data + 5;
        }
      } 
    }
    // OPTIONS
    case 'O': {
      if(data_len < 7) return NULL;
      if(*(data+1) != 'P' || *(data+2) != 'T' || *(data+3) != 'I' || *(data+4) != 'O' || *(data+5) != 'N' || *(data+6) != 'S') return NULL;
      *method = OPTIONS;
      return data + 7;
    }
    // HEAD
    case 'H': {
      if(data_len < 4) return NULL;
      if(*(data+1) != 'E' || *(data+2) != 'A' || *(data+3) != 'D') return NULL;
      *method = HEAD;
      return data + 4;
    }
    // DELETE
    case 'D': {
      if(data_len < 6) return NULL;
      if(*(data+1) != 'E' || *(data+2) != 'L' || *(data+3) != 'E' || *(data+4) != 'T' || *(data+5) != 'E') return NULL;
      *method = DELETE;
      return data + 6;
    }
    // TRACE
    case 'T': {
      if(data_len < 5) return NULL;
      if(*(data+1) != 'R' || *(data+2) != 'A' || *(data+3) != 'C' || *(data+4) != 'E') return NULL;
      *method = TRACE;
      return data + 6;
    }
    // CONNECT
    case 'C': {
      if(data_len < 7) return NULL;
      if(*(data+1) != 'O'|| *(data+2) != 'N'|| *(data+3) != 'N'|| *(data+4) != 'E'|| *(data+5) != 'C'|| *(data+6) != 'T') return NULL;
      *method = CONNECT;
      return data + 7;
    }
  }
  return NULL;
}

char* _mh_parse_path(char* data, char* data_end, char* path, unsigned int* path_len) {
  unsigned int limit = MIN(data_end - data, *path_len);
  unsigned int i = 0;
  for(; i < limit && data[i] != ' '; i++) {
    path[i] = data[i];
  }
  *path_len = &data[i] - data;
  for(; data[i] != ' '; i++);
  return &data[i];
}

char* _mh_parse_version(char* data, char* data_end, mh_version* version) {
  if(data_end < data + 8) return NULL;
  EXPECT_NO_CHECK('H');
  EXPECT_NO_CHECK('T');
  EXPECT_NO_CHECK('T');
  EXPECT_NO_CHECK('P');
  EXPECT_NO_CHECK('/');
  EXPECT_NO_CHECK('1');
  EXPECT_NO_CHECK('.');
  switch(*data) {
    case '0': {
      *version = HTTP_1;
      break;
    } 
    case '1': {
      *version = HTTP_1_1;
      break;
    }
    default: {
      return NULL;
    }
  }
  return data + 1;
}

char* mh_parse_request_first_line(char* data, char* data_end, mh_method* method, char* path, unsigned int* path_len, mh_version* version) {
  if(data_end < data) return NULL;
  data = _mh_parse_method(data, data_end - data, method);
  UNTIL_NOT(' ');
  data = _mh_parse_path(data, data_end, path, path_len);
  UNTIL_NOT(' ');
  data = _mh_parse_version(data, data_end, version);
  EXPECT_NEWLINE();
}

char* mh_parse_headers(char* data, char* data_end, mh_header* headers, unsigned int* num_headers) {
  unsigned int header_counter = 0;
  for(; header_counter < *num_headers; header_counter++) {
    // look for ':'
    // if '\r' or '\n' is found, return NULL
    char* first_non_space = NULL;
    headers[header_counter].header_key_begin = data;
    for(; data < data_end; data++) {
      CHECK_EOF();
      switch(*data) {
        // newline? then should be value with empty header name
        case '\r': 
        case '\n': {
          if(!first_non_space) return data + 1;
          headers[header_counter].header_key_len = 0;
          headers[header_counter].header_value_len = data - headers[header_counter].header_key_begin;
          headers[header_counter].header_value_begin = first_non_space;
          *num_headers = header_counter+1;
        }
        case ':': {
          if(!first_non_space) return NULL;
          headers[header_counter].header_key_len = data - headers[header_counter].header_key_begin;
          goto space_parsing;
        }
        case ' ': {
          if(first_non_space) return NULL;
          headers[header_counter].header_key_begin++;
          break;
        }
        default:
          first_non_space = data;
      }
    }
space_parsing:
    // look for '\r' or '\n'
    UNTIL_NOT(' ');
    headers[header_counter].header_value_begin = data;
    for(; data < data_end; data++) {
      CHECK_EOF();
      switch(*data) {
        case '\r': {
          if(data_end - data < 2) return NULL;
          if(*(data+1) != '\n') return NULL;
          headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
          data++;
          break;
        }
        case '\n': {
          headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
          break;
        }
      }
    }
  }
  *num_headers = header_counter;
  return data;
}

char* mh_parse_request(char* data, char* data_end, mh_method* method, char* path, unsigned int* path_len, mh_version* version, mh_header* headers, unsigned int* num_headers) {
  data = mh_parse_request_first_line(data, data_end, method, path, path_len, version);
  return mh_parse_headers(data, data_end, headers, num_headers);
}
