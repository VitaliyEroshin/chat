#include "filesystem.hpp"
#include <fstream>
#include <sstream>
#include <iostream>


std::string fs::loadContent(const std::string& path) {
  std::ifstream index;
  index.open(path);

  std::stringstream ss;
  ss << index.rdbuf();
  return ss.str();
}