#pragma once
#include <iostream>
#include <string>
#include "entities.hpp"

class Compression {
  typedef std::string bytes;
  bytes compress(const Object& object);
  Object decompress(const bytes& bytes);
};