#pragma once
#include <string>

typedef u_int64_t objectid_t;
typedef u_int16_t userid_t;
typedef std::string object_message_t;
typedef u_int16_t code_t;

struct Object {
  enum class Type: char {
    unknown = 0,
    text = 1,
    loginAttempt = 2,
    returnCode = 3,
  };

  objectid_t id;
  int timestamp;
  objectid_t parentId;
  userid_t author;
  code_t ret;
  Type type;
  object_message_t message;
};
