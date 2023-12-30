#include "minhttp.h"

#define NULL 0

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

char* _mh_parse_method(char* data, uint32_t data_len, mh_method* method) {
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
          if(data_len == 3 || *(data+2) != 'S' || *(data+3) != 'T') return NULL;
          *method = POST;
          return data + 4;
        }
        case 'A': {
          if(data_len == 4 || *(data+2) != 'T' || *(data+3) != 'C' || *(data+4) != 'H') return NULL;
          * method = PATCH;
          return data + 5;
        }
      } 
    }
    // OPTIONS
    case 'O': {
      if(data_len < 7 || *(data+1) != 'P' || *(data+2) != 'T' || *(data+3) != 'I' || *(data+4) != 'O' || *(data+5) != 'N' || *(data+6) != 'S') return NULL;
      *method = OPTIONS;
      return data + 7;
    }
    // HEAD
    case 'H': {
      if(data_len < 4 || *(data+1) != 'E' || *(data+2) != 'A' || *(data+3) != 'D') return NULL;
      *method = HEAD;
      return data + 4;
    }
    // DELETE
    case 'D': {
      if(data_len < 6 || *(data+1) != 'E' || *(data+2) != 'L' || *(data+3) != 'E' || *(data+4) != 'T' || *(data+5) != 'E') return NULL;
      *method = DELETE;
      return data + 6;
    }
    // TRACE
    case 'T': {
      if(data_len < 5 || *(data+1) != 'R' || *(data+2) != 'A' || *(data+3) != 'C' || *(data+4) != 'E') return NULL;
      *method = TRACE;
      return data + 6;
    }
    // CONNECT
    case 'C': {
      if(data_len < 7 || *(data+1) != 'O'|| *(data+2) != 'N'|| *(data+3) != 'N'|| *(data+4) != 'E'|| *(data+5) != 'C'|| *(data+6) != 'T') return NULL;
      *method = CONNECT;
      return data + 7;
    }
  }
  return NULL;
}

char* _mh_parse_path(char* data, char* data_end, char* path, uint32_t* path_len) {
  uint32_t limit = MIN(data_end - data, *path_len);
  uint32_t i = 0;
  for(; i < limit && data[i] != ' '; i++) {
    path[i] = data[i];
  }
  *path_len = &data[i] - data;
  for(; data[i] != ' '; i++);
  return &data[i];
}


char* _mh_parse_phrase(char* data, char* data_end, char* phrase, uint32_t* phrase_len) {
  uint32_t limit = MIN(data_end - data, *phrase_len);
  uint32_t i = 0;
  for(; i < limit && data[i] != '\r' && data[i] != '\n'; i++) {
    phrase[i] = data[i];
  }
  *phrase_len = &data[i] - data;
  return &data[i];
}

char* _mh_parse_status_code(char* data, char* data_end, uint16_t* status) {
  // buffer size left
  if(data_end - data < 3) return NULL;
  // are digits valid?
  if((data[0] < '1' || '5' < data[0]) || (data[1] < '0' || '9' < data[1]) || (data[2] < '0' || '9' < data[2])) return NULL;
  *status = 100 * (data[0] - '0') + 10 * (data[1] - '0') + data[2] - '0';
  return data + 3;
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

char* mh_parse_request_first_line(char* data, char* data_end, mh_method* method, char* path, uint32_t* path_len, mh_version* version) {
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

char* mh_parse_response_first_line(char* data, char* data_end, mh_version* version, uint16_t* status, char* phrase, uint32_t* phrase_len) {
  if(data_end < data) return NULL;
  data = _mh_parse_version(data, data_end, version);
  if(!data) return NULL;
  UNTIL_NOT(' ');
  data = _mh_parse_status_code(data, data_end, status);
  if(!data) return NULL;
  if(*data != '\r' && *data != '\n' && *data != ' ') return NULL;
  UNTIL_NOT(' ');
  data = _mh_parse_phrase(data, data_end, phrase, phrase_len);
  if(!data) return NULL;
  EXPECT_NEWLINE();
  return data;
}

enum __MH_HEADER_PARSER_STATE {
  LINE_START = 0,
  FIRST_STRING = 1,
  AFTER_KEY = 2,
  DURING_VALUE = 3,
  AFTER_WHITESPACE = 4,
  DONE = 5,
  STATE_ERROR = 6
};

// enum __MH_HEADER_PARSER_TOKEN {
//   NEWLINE = 0,
//   WHITESPACE = 1,
//   COLON = 2,
//   OTHER = 3,
// };

enum __MH_HEADER_PARSER_TOKEN {
  NEWLINE = 0,
  WHITESPACE = 1,
  COLON = 2,
  OTHER = 3,
  TOKEN_ERROR = 4,
};

enum __MH_HEADER_PARSER_ACTION {
  START_KEY = 1,
  END_KEY = 2,
  START_VALUE = 4,
  END_VALUE = 8,
  EMPTY_VALUE = 16,
  NOTHING = 32,
  NEXT_HEADER = 64,
  END_VALUE_AND_NEXT_HEADER = 128,
  ACTION_ERROR = 0
};

// |        x           |  Newline   |      Whitespace      |    Colon     |       Other       |
// |--------------------|------------|----------------------|--------------|-------------------|
// |Line Start          |    DONE    |                      |              |   First String    |
// |First String        |            |                      |  After Key   |   First String    |
// |After Key           | Line Start |      After Key       |              |   During Value    |
// |During Value        | Line Start |   After Whitespace   | During Value |   During Value    |
// |After Whitespace    | Line Start |   After Whitespace   | During Value |   During Value    |

enum __MH_HEADER_PARSER_STATE state_token_to_state[5][4] = {
  {DONE,        STATE_ERROR,      STATE_ERROR,  FIRST_STRING}, // Line Start
  {STATE_ERROR, STATE_ERROR,      AFTER_KEY,    FIRST_STRING}, // First String
  {LINE_START,  AFTER_KEY,        STATE_ERROR,  DURING_VALUE}, // After Key
  {LINE_START,  AFTER_WHITESPACE, DURING_VALUE, DURING_VALUE}, // During Value
  {LINE_START,  AFTER_WHITESPACE, DURING_VALUE, DURING_VALUE} // After Whitespace
};

enum __MH_HEADER_PARSER_ACTION state_token_to_action[5][4] = {
  {NOTHING,        ACTION_ERROR,      ACTION_ERROR,  START_KEY}, // Line Start
  {ACTION_ERROR, ACTION_ERROR,      END_KEY,    NOTHING}, // First String
  {NEXT_HEADER,  NOTHING, ACTION_ERROR,  START_VALUE}, // After Key
  {END_VALUE_AND_NEXT_HEADER,  END_VALUE, NOTHING, NOTHING}, // During Value
  {NEXT_HEADER,  NOTHING, NOTHING, NOTHING} // After Whitespace
};

static inline char* _mh_parse_headers_token(char* data, char* data_end, enum __MH_HEADER_PARSER_TOKEN* token) { 
  CHECK_EOF();
  switch(*data) {
    default: {
      *token = OTHER;
      return data + 1;
    }
    // whitespace
    case ' ':
    case '\t': {
      *token = WHITESPACE;
      return data + 1;
    }
    // newline
    case '\r': {
      if(data + 1 == data_end || *(data + 1) != '\n') return NULL;
      data++;
    }
    case '\n': {
      *token = NEWLINE;
      return data + 1;
    }
    case ':': {
      *token = COLON;
      return data + 1;
    }
  }
  return NULL;
}

// enum __MH_HEADER_PARSER_ACTION state_token_to_action[5][4] = {
//   {NOTHING,        ACTION_ERROR,      ACTION_ERROR,  START_KEY}, // Line Start
//   {ACTION_ERROR, ACTION_ERROR,      END_KEY,    NOTHING}, // First String
//   {NEXT_HEADER,  NOTHING, ACTION_ERROR,  START_VALUE}, // After Key
//   {END_VALUE_AND_NEXT_HEADER,  END_VALUE, NOTHING, NOTHING}, // During Value
//   {NEXT_HEADER,  NOTHING, NOTHING, NOTHING} // After Whitespace
// };

char* _mh_parse_headers(char* data, char* data_end, mh_header* headers, uint32_t* num_headers) {
  enum __MH_HEADER_PARSER_STATE state = LINE_START;
  enum __MH_HEADER_PARSER_TOKEN token;
  uint32_t header_counter = 0;
  while(header_counter < *num_headers && state != DONE) {
    char* next_data = NULL;
    if(!(next_data = _mh_parse_headers_token(data, data_end, &token))) return NULL;

    enum __MH_HEADER_PARSER_ACTION action = state_token_to_action[state][token];

    // switch(action) {
    //   case START_KEY: {
    //       headers[header_counter].header_key_begin = data;
    //       break;
    //   }
    //   case END_KEY: {
    //       headers[header_counter].header_key_len = data - headers[header_counter].header_key_begin;
    //       break;
    //   }
    //   case START_VALUE: {
    //       headers[header_counter].header_value_begin = data;
    //       break;
    //   }
    //   case END_VALUE: {
    //       headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
    //       break;
    //   }
    //   case EMPTY_VALUE: {
    //       headers[header_counter].header_value_begin = NULL;
    //       headers[header_counter].header_value_len = 0;
    //       break;
    //   }
    //   case END_VALUE_AND_NEXT_HEADER: {
    //       headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
    //   }
    //   case NEXT_HEADER: {
    //       header_counter++;
    //       break;
    //   }
    //   case ACTION_ERROR: {
    //       return NULL;
    //   }
    //   case NOTHING: {}
    // }

    switch(state) {
      case LINE_START: {
        switch(token) {
          case OTHER: {
            headers[header_counter].header_key_begin = data;
            break;
          }
          case NEWLINE: {
            break;
          }
          default: return NULL;
        }
        break;
      }
      case FIRST_STRING: {
        switch(token) {
          case COLON: {
            headers[header_counter].header_key_len = data - headers[header_counter].header_key_begin;
            break;
          }
          case OTHER: break;
          default: return NULL;
        }
        break;
      }
      case AFTER_KEY: {
        switch(token) {
          case WHITESPACE: break;
          case NEWLINE: {
            headers[header_counter].header_value_begin = NULL;
            headers[header_counter].header_value_len = 0;
            header_counter++;
            break;
          }
          case OTHER: {
            headers[header_counter].header_value_begin = data;
            break;
          }
          default: return NULL;
        }
        break;
      }
      case DURING_VALUE: {
        switch(token) {
          case NEWLINE:{
            headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
            header_counter++;
            break;
          }
          case WHITESPACE: {
            headers[header_counter].header_value_len = data - headers[header_counter].header_value_begin;
            break;
          }
          case COLON: break;
          case OTHER: break;
          default: return NULL;
        }
        break;
      }
      case AFTER_WHITESPACE: {
        switch(token) {
          case WHITESPACE: break;
          case NEWLINE: {
            header_counter++;
            break;
          }
          case COLON:
          case OTHER: {
            break;
          }
          default: return NULL;
        }
        break;
      }
      default: return NULL;
    }
    state = state_token_to_state[state][token];
    data = next_data;
  }
  *num_headers = header_counter;
  return data;
}

char* mh_parse_headers(char* data, char* data_end, mh_header* headers, uint32_t* num_headers) {
  data = _mh_parse_headers(data, data_end, headers, num_headers);
  if(!data) *num_headers = 0;
  return data;
}
