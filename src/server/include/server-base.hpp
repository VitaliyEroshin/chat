#pragma once

#include "types.hpp"
#include "objects.hpp"
#include "storage.hpp"

#include <functional> // std::function
#include <sstream> // std::stringstream

struct ConnectionBase {
public:
  enum Status {
    unauthorized,
    inmenu,
    inchat,
    inprofile
  };

  userid_t user{};
  chatid_t chat{};
  Status status = unauthorized;
};