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
  static char get_type_id(const Object& object);
  static Object::Type from_type_id(char id);

  template<typename T>
  bytes to_bytes(T id);

  template<typename T>
  T from_bytes(const bytes& b);

public:
  bytes encode(const Object& object) override;
  Object decode(const bytes& bytes) override;
};