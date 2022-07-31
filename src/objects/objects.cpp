#include "objects.hpp"

bool bit(char x, int i) {
    return (x & (1 << i)) != 0;
}

void set(char& x, int i, bool value) {
    if (value) 
        x |= (1 << i);
    else if (bit(x, i)) 
        x ^= (1 << i);
}

void Object::set_id(int id_) {
    id = id_;
    set(attributes, 0, true);
}

void Object::set_author(int author_) {
    author = author_;
    set(attributes, 1, true);
}

void Object::set_timestamp(int64_t timestamp_) {
    timestamp = timestamp_;
    set(attributes, 2, true);
}

void Object::set_thread(int thread_) {
    thread = thread_;
    set(attributes, 3, true);
}

void Object::set_reply(int reply_) {
    reply = reply_;
    set(attributes, 4, true);
}

void Object::set_prev(int prev_) {
    prev = prev_;
    set(attributes, 5, true);
}

void Object::set_next(int next_) {
    next = next_;
    set(attributes, 6, true);
}

void Object::set_return_code(int code_) {
    code = code_;
    set(attributes, 7, true);
}

bool Object::has_id() const
{ return bit(attributes, 0); }

bool Object::has_author() const
{ return bit(attributes, 1); }

bool Object::has_timestamp() const
{ return bit(attributes, 2); }

bool Object::has_thread() const
{ return bit(attributes, 3); }

bool Object::has_reply() const
{ return bit(attributes, 4); }

bool Object::has_prev() const
{ return bit(attributes, 5); }

bool Object::has_next() const
{ return bit(attributes, 6); }

bool Object::has_return_code() const
{ return bit(attributes, 7); }

bool Object::has_return_code(int code_) const
{ return has_return_code() && code == code_; }

std::string Object::info() const {
  std::string s = "[\n";
  s += "  content: \"" + content + "\"\n";
  if (has_prev())
    s += "  prev_id: " + std::to_string(prev) + "\n";

  if (has_next())
    s += "  next_id: " + std::to_string(next) + "\n";
  
  if (has_id())
    s += "  self_id: " + std::to_string(id) + "\n";

  if (has_return_code())
    s += "  return_code: " + std::to_string(code) + "\n";

  s += "]\n";
  return s;
}