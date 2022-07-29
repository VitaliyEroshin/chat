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
  int64_t timestamp;
  int thread;
  int reply;
  int prev;
  int next;
  int code;
  std::string content;
  Type type;

  void set_id(int id_);
  void set_author(int author_);
  void set_timestamp(int64_t timestamp_);
  void set_thread(int thread_);
  void set_reply(int reply_);
  void set_prev(int prev_);
  void set_next(int next_);
  void set_return_code(int code_);

  bool has_id() const;
  bool has_author() const;
  bool has_timestamp() const;
  bool has_thread() const;
  bool has_reply() const;
  bool has_prev() const;
  bool has_next() const;
  bool has_return_code() const;
  
  bool has_return_code(int code_) const;

  std::string info() const;
};
