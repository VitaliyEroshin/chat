#include "encoder.hpp"

char Encoder::getTypeId(const Object& object) {
  if (object.type == Object::Type::text) {
    return 1;
  }

  return 0;
}

Object::Type Encoder::fromTypeId(char id) {
  if (id == 1) {
    return Object::Type::text;
  }

  return Object::Type::unknown;
}

template<typename T>
Encoder::bytes Encoder::toBytes(T id) {
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
T Encoder::fromBytes(const Encoder::bytes& b) {
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

Encoder::bytes Encoder::encode(const Object& object) {
  std::string s;
  s.push_back(getTypeId(object));
  s += toBytes(object.id);
  // s += toBytes(object.author);
  // s += toBytes(object.timestamp);
  // s += toBytes(object.parentId);
  s += object.message;
  return s;
}

Object Encoder::decode(const Encoder::bytes& bytes) {
  Object obj;
  
  char type = bytes[0];
  int ptr = 1;
  obj.type = fromTypeId(type);
  obj.id = fromBytes<objectid_t>(bytes.substr(ptr, sizeof(objectid_t)));
  ptr += sizeof(objectid_t);
  for (int i = ptr; i < bytes.size(); ++i) {
    obj.message += bytes[i];
  }
  return obj;
}