#pragma once
#include <string>
#include "types.hpp"

struct Object {
  enum class Type: char {
    unknown = 0,
    text = 1,
    loginAttempt = 2,
    returnCode = 3,
    command = 4,
  };

  
  char attributes = 0;
  int id;
  int author;
  int timestamp;
  int thread;
  int reply;
  int prev;
  int next;
  int code;
  std::string content;
  Type type;

  void setId(int id_);
  void setAuthor(int author_);
  void setTimestamp(int timestamp_);
  void setThread(int thread_);
  void setReply(int reply_);
  void setPrev(int prev_);
  void setNext(int next_);
  void setReturnCode(int code_);

  bool hasId();
  bool hasAuthor();
  bool hasTimestamp();
  bool hasThread();
  bool hasReply();
  bool hasPrev();
  bool hasNext();
  bool hasReturnCode(int code_);
};
