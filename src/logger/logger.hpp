#pragma once
#include <vector>
#include <iostream>
#include <ostream>

class Logger {
public:
  Logger(std::initializer_list<std::ostream*> streams);

  template<typename T>
  Logger& operator<<(const T& object) {
    for (auto &out : s) {
      *out << object;
    }
    return *this;
  }

  typedef std::basic_ostream<char, std::char_traits<char> > outType;
  typedef outType& (*endlManipulator)(outType&);

  Logger& operator<<(endlManipulator manip) {
    for (auto &out : s) {
      manip(*out);
    }
    return *this;
  }

private:
  std::vector<std::ostream*> s;
};