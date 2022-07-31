#include "encoder.hpp"

char StrEncoder::get_type_id(const Object& object) {
  if (object.type == Object::Type::text) {
    return 1;
  } 

  if (object.type == Object::Type::loginAttempt) {
    return 2;
  }

  if (object.type == Object::Type::returnCode) {
    return 3;
  }

  if (object.type == Object::Type::command) {
    return 4;
  }
  return 0;
}

Object::Type StrEncoder::from_type_id(char id) {
  if (id == 1) {
    return Object::Type::text;
  }

  if (id == 2) {
    return Object::Type::loginAttempt;
  }

  if (id == 3) {
    return Object::Type::returnCode;
  }

  if (id == 4) {
    return Object::Type::command;
  }

  return Object::Type::unknown;
}

template<typename T>
Encoder::bytes StrEncoder::to_bytes(T id) {
  const auto size = sizeof(T);
  char result[size];
    
  std::copy(
    reinterpret_cast<char*>(&id),
    reinterpret_cast<char*>(&id) + size,
    &result[0]
  );
  bytes b(result);
  b.resize(size);
  return b;
}

template<typename T>
T StrEncoder::from_bytes(const Encoder::bytes& b) {
  const auto size = sizeof(T);
  T result;
  const char* r_bytes = b.c_str();

  std::copy(
    &r_bytes[0],
    &r_bytes[0] + size,
    reinterpret_cast<char*>(&result)
  );

  return result;
}

#include <algorithm> // reverse
std::string remove_zeroes(std::string s) {
  std::string result;
  std::reverse(s.begin(), s.end());
  auto ptr = reinterpret_cast<const unsigned char*>(&s[0]);

  int carry = 0;
  for (int i = 0; i < s.size(); ++i) {
    int x = ptr[i];
    carry = carry * 256 + x;
    while (carry >= 255) {
      result.push_back(carry % 255 + 1);
      carry /= 255;
    }
  }

  if (carry != 0) {
    result.push_back(carry + 1);
  }

  return result;
}

std::string add_zeroes(std::string s) {
  std::string result;
  std::reverse(s.begin(), s.end());
  auto ptr = reinterpret_cast<const unsigned char*>(&s[0]);

  int carry = 0;

  for (int i = 0; i < s.size(); ++i) {
    int x = ptr[i];
    carry = carry * 255 + (x - 1);
    while (carry >= 256) {
      result.push_back(carry % 256);
      carry /= 256;
    }
  }

  if (carry != 0) {
    result.push_back(carry);
  }

  return result;
}


Encoder::bytes StrEncoder::encode(const Object& object) {
  std::string s = "";
  s.push_back(get_type_id(object));
  s.push_back(object.attributes);
  if (object.has_id())
    s += to_bytes(object.id);

  if (object.has_author())
    s += to_bytes(object.author);

  if (object.has_timestamp())
    s += to_bytes(object.timestamp);

  if (object.has_thread())
    s += to_bytes(object.thread);

  if (object.has_reply())
    s += to_bytes(object.reply);

  if (object.has_prev())
    s += to_bytes(object.prev);

  if (object.has_next())
    s += to_bytes(object.next);

  if (object.has_return_code())
    s += to_bytes(object.code);

  s += object.content;
  s.push_back(1);
  s = remove_zeroes(s);
  return s;
}

Object StrEncoder::decode(Encoder::bytes bytes) {
  Object object;
  bytes = add_zeroes(bytes);

  char type = bytes[0];
  object.type = from_type_id(type);
  object.attributes = bytes[1];

  int ptr = 2;
  if (object.has_id()) {
    object.id = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.has_author()) {
    object.author = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }
    
  if (object.has_timestamp()) {
    object.timestamp = from_bytes<int64_t>(bytes.substr(ptr, sizeof(int64_t)));
    ptr += sizeof(int64_t);
  }

  if (object.has_thread()) {
    object.thread = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.has_reply()) {
    object.reply = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.has_prev()) {
    object.prev = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.has_next()) {
    object.next = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.has_return_code()) {
    object.code = from_bytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  for (int i = ptr; i < bytes.size(); ++i) {
    object.content += bytes[i];
  }
  object.content.pop_back();
  
  return object;
}