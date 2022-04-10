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
  s.push_back(object.attributes);
  if (object.hasId())
    s += toBytes(object.id);

  if (object.hasAuthor())
    s += toBytes(object.author);

  if (object.hasTimestamp())
    s += toBytes(object.timestamp);

  if (object.hasThread())
    s += toBytes(object.thread);

  if (object.hasReply())
    s += toBytes(object.reply);

  if (object.hasPrev())
    s += toBytes(object.prev);

  if (object.hasNext())
    s += toBytes(object.next);

  if (object.hasReturnCode())
    s += toBytes(object.code);

  s += object.content;
  return s;
}

Object StrEncoder::decode(const Encoder::bytes& bytes) {
  Object object;
  
  char type = bytes[0];
  object.type = fromTypeId(type);
  object.attributes = bytes[1];

  int ptr = 2;
  if (object.hasId()) {
    object.id = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.hasAuthor()) {
    object.author = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }
    
  if (object.hasTimestamp()) {
    object.timestamp = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.hasThread()) {
    object.thread = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.hasReply()) {
    object.reply = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.hasPrev()) {
    object.prev = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.hasNext()) {
    object.next = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }

  if (object.hasReturnCode()) {
    object.code = fromBytes<int>(bytes.substr(ptr, sizeof(int)));
    ptr += sizeof(int);
  }
  
  return object;
}