#include "minhttp.h"


#define NULL 0
#define uint8_t unsigned char

#include <stdio.h>

#define MIN(a, b) (a < b ? a : b)
#if __GNUC__ >= 3
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define CHECK_EOF() if(data == data_end || data == 0x00) return NULL;

#define EXPECT_NO_CHECK(x) CHECK_EOF(); if(unlikely(*data != x)) return NULL; data++;

#define EXPECT(x) CHECK_EOF(); EXPECT_NO_CHECK(x); if(data_end <= data) return NULL;

#define UNTIL_NOT(x) for(; data != data_end && *data == x; data++) CHECK_EOF();


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
  if(!data) return NULL;
  UNTIL_NOT(' ');
  data = _mh_parse_path(data, data_end, path, path_len);
  if(!data) return NULL;
  UNTIL_NOT(' ');
  data = _mh_parse_version(data, data_end, version);
  if(!data) return NULL;
  EXPECT_NEWLINE();
}

enum __MH_HEADER_PARSER_STATE {
  BEFORE_FIRST_SPACE = ' ',
  BEFORE_KEY = 'k',
  FIRST_STRING = 's',
  AFTER_KEY = 'K',
  DURING_VALUE = 'V',
  DONE = 'D'
};

enum __MH_HEADER_PARSER_TOKEN {
  SPACE = ' ',
  COLON = ':',
  NEWLINE = '\n',
  OTHER = 'O',
  ERROR = 0
};

static inline char* _mh_parse_headers_token(char* data, char* data_end, enum __MH_HEADER_PARSER_TOKEN* token) { 
  CHECK_EOF();
  switch(*data) {
    default: {
      *token = OTHER;
      return data + 1;
    }
    // newline
    case '\r': {
      if(data + 1 == data_end) return (void*)ERROR;
      if(*(data + 1) != '\n') return (void*)ERROR;
      data++;
    }
    case SPACE:
    case NEWLINE:
    case COLON: {}
  }
  *token = *data;
  return data + 1;
}
char* _mh_parse_headers(char* data, char* data_end, mh_header* headers, unsigned int* num_headers) {
  enum __MH_HEADER_PARSER_STATE state = BEFORE_FIRST_SPACE;
  enum __MH_HEADER_PARSER_TOKEN token;
  unsigned int header_counter = 0;
  while(header_counter < *num_headers && state != DONE) {
    char* next_data = NULL;
    if(!(next_data = _mh_parse_headers_token(data, data_end, &token))) return NULL;
    switch(state) {
      case BEFORE_FIRST_SPACE: {
        switch(token) {
          case OTHER: {
            state = FIRST_STRING;
            headers[header_counter].header_key_begin = data;
            break;
          }
          case SPACE: {
            state = BEFORE_KEY;
            break;
          }
          case NEWLINE: {
            state = DONE;
            break;
          }
          default: return (void*)ERROR;
        }
        break;
      }
      case BEFORE_KEY: {
        switch(token) {
          case SPACE: break;
          case OTHER: {
            state = FIRST_STRING;
            headers[header_counter].header_key_begin = data;
            break;
          }
          default: return (void*)ERROR;
        }
        break;
      }
      case FIRST_STRING: {
        switch(token) {
          // case NEWLINE: {
          //   state = BEFORE_FIRST_SPACE;
          //   headers[header_counter].header_value_begin = headers[header_counter].header_key_begin;
          //   headers[header_counter].header_key_begin = NULL;
          //   headers[header_counter].header_key_len = 0;
          //   headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
          //   header_counter++;
          //   break;
          // }
          case COLON: {
            state = AFTER_KEY;
            headers[header_counter].header_key_len = data - headers[header_counter].header_key_begin;
            break;
          }
          case OTHER: break;
          default: return (void*)ERROR;
        }
        break;
      }
      case AFTER_KEY: {
        switch(token) {
          case SPACE: break;
          case NEWLINE: {
            state = BEFORE_FIRST_SPACE;
            headers[header_counter].header_value_begin = NULL;
            headers[header_counter].header_value_len = 0;
            header_counter++;
            break;
          }
          case OTHER: {
            state = DURING_VALUE;
            headers[header_counter].header_value_begin = data;
            break;
          }
          default: return (void*)ERROR;
        }
        break;
      }
      case DURING_VALUE: {
        switch(token) {
          case NEWLINE:{
            state = BEFORE_FIRST_SPACE;
            headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
            header_counter++;
            break;
          }
          case OTHER: break;
          default: return (void*)ERROR;
        }
        break;
      }
      default: return (void*)ERROR;
    }
    data = next_data;
  }
  *num_headers = header_counter;
  return data;
}

char* mh_parse_headers(char* data, char* data_end, mh_header* headers, unsigned int* num_headers) {
  data = _mh_parse_headers(data, data_end, headers, num_headers);
  if(!data) *num_headers = 0;
  return data;
}

char* mh_parse_request(char* data, char* data_end, mh_method* method, char* path, unsigned int* path_len, mh_version* version, mh_header* headers, unsigned int* num_headers) {
  data = mh_parse_request_first_line(data, data_end, method, path, path_len, version);
  return mh_parse_headers(data, data_end, headers, num_headers);
}
