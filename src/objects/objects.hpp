#pragma once
#include <string>
#include "types.hpp"

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
