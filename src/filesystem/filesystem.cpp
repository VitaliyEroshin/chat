#include "filesystem.hpp"


std::string fs::loadContent(const std::string& path) {
  std::ifstream index;
  index.open(path);

  std::stringstream ss;
  ss << index.rdbuf();
  return ss.str();
}

void fs::Config::load(const std::string& path) {
  std::ifstream in;
  in.open(path);
  while (in){
    std::string argumentName;
    in >> argumentName;
    if (argumentName.empty()) {
      break;
    }
    std::string argumentType;
    std::string argumentValue;
    char eq;
    in >> eq >> argumentType >> argumentValue;
    std::any value;
    if (argumentType == "int") {
      try {
        value = std::stoi(argumentValue);
      } catch (...) {
        log << "Make sure, that " << argumentName << " has integer value. \n";
      }
    } else if (argumentType == "string") {
      value = argumentValue;
    }

    parameters[argumentName] = value;
  }
}

fs::Config::Config(Logger& log): log(log) {};