#pragma once

#include <string>
#include <any>
#include <map>
#include <string>

namespace ServerConfig {
  std::map<std::string, std::any> get_default_values();
}

namespace ClientConfig {
  std::map<std::string, std::any> get_default_values();
};