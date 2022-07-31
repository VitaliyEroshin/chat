#pragma once
#include <string>
#include <map>
#include <any>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>
#include <filesystem>
#include "logger.hpp"


namespace fs {
  std::string load_content(const std::string& path);
    
  int get_file_count(const std::string& path);

  class Config {
  public:
    Config(Logger& log);
    Config(Logger& log, const std::string& path);
    void load(const std::string& path, const std::map<std::string, std::any>& default_values = {});

    template<typename T>
    const T& get(const std::string& name) {
      if (parameters.count(name)) {
        return std::any_cast<const T&>(parameters[name]);
      } else if (default_parameters.count(name)) {
        return std::any_cast<const T&>(default_parameters[name]);
      }
      log << "No argument with " << name << " found. \n";
      assert(false);
    }

  private:
    Logger& log;
    std::map<std::string, std::any> default_parameters;
    std::map<std::string, std::any> parameters;
  };
}
