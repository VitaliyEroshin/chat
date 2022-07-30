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

using handler_t = std::function<
void(std::stringstream&, ConnectionBase&, const Object&,
const std::function<void(const Object&)>&, Storage&)>;
