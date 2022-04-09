#pragma once
#include "objects.hpp"
#include <iostream>

class Encoder {
public:
  typedef std::string bytes;
  virtual bytes encode(const Object& object) = 0;
  virtual Object decode(const bytes& bytes) = 0;
};

class StrEncoder: public Encoder {
private:
  static char getTypeId(const Object& object);
  static Object::Type fromTypeId(char id);

  template<typename T>
  bytes toBytes(T id);

  template<typename T>
  T fromBytes(const bytes& b);

public:
  bytes encode(const Object& object) override;
  Object decode(const bytes& bytes) override;
};