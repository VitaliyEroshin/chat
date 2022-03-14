#pragma once
#include "objects.hpp"
#include <iostream>
class Encoder {
public:
  typedef std::string bytes;

private:
  char getTypeId(const Object& object);
  Object::Type fromTypeId(char id);

  template<typename T>
  bytes toBytes(T id);

  template<typename T>
  T fromBytes(const bytes& b);
  
public:
  bytes encode(const Object& object);
  Object decode(const bytes& bytes);
};