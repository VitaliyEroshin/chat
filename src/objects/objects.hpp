#pragma once
#include <string>

typedef u_int64_t objectid_t;
typedef std::string object_author_t;
typedef std::string object_message_t;

struct Object {
  enum class Type: char {
    unknown = 0,
    text = 1
  };

  objectid_t id;
  int timestamp;
  objectid_t parentId;
  object_author_t author;

  Type type;

  object_message_t message;
};
