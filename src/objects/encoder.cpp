#include "encoder.hpp"

char StrEncoder::getTypeId(const Object& object) {
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

Object::Type StrEncoder::fromTypeId(char id) {
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
Encoder::bytes StrEncoder::toBytes(T id) {
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
T StrEncoder::fromBytes(const Encoder::bytes& b) {
  const auto size = sizeof(T);
  T result;
  const char* rawBytes = b.c_str();

  std::copy(
    &rawBytes[0],
    &rawBytes[0] + size,
    reinterpret_cast<char*>(&result)
  );

  return result;
}

Encoder::bytes StrEncoder::encode(const Object& object) {
  std::string s = "";
  s.push_back(getTypeId(object));

  if (object.type == Object::Type::loginAttempt || object.type == Object::Type::command) {
    s += object.message;
    return s;
  }
  if (object.type == Object::Type::returnCode) {
    s += toBytes(object.ret);
    return s;
  }

  s += toBytes(object.id);
  // s += toBytes(object.author);
  // s += toBytes(object.timestamp);
  // s += toBytes(object.parentId);
  s += object.message;
  return s;
}

Object StrEncoder::decode(const Encoder::bytes& bytes) {
  Object obj;
  
  char type = bytes[0];
  obj.type = fromTypeId(type);

  int ptr = 1;
  if (obj.type == Object::Type::returnCode) {
    obj.ret = fromBytes<code_t>(bytes.substr(ptr, sizeof(code_t)));
    return obj;
  }

  if (obj.type != Object::Type::loginAttempt) {
    obj.id = fromBytes<objectid_t>(bytes.substr(ptr, sizeof(objectid_t)));
    ptr += sizeof(objectid_t);
  }

  for (int i = ptr; i < bytes.size(); ++i) {
    obj.message += bytes[i];
  }
  return obj;
}