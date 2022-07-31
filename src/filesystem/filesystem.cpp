#include "filesystem.hpp"


std::string fs::load_content(const std::string& path) {
  std::ifstream index;
  index.open(path);

  std::stringstream ss;
  ss << index.rdbuf();
  return ss.str();
}

void fs::Config::load(const std::string& path, const std::map<std::string, std::any>& default_values) {
  default_parameters = default_values;

  std::ifstream in;
  in.open(path);
  while (in){
    std::string arg_name;
    in >> arg_name;
    if (arg_name.empty()) {
      break;
    }
    std::string arg_type;
    std::string arg_value;
    char eq;
    in >> eq >> arg_type >> arg_value;
    std::any value;
    if (arg_type == "int") {
      try {
        value = std::stoi(arg_value);
      } catch (...) {
        log << "Make sure, that " << arg_name << " has integer value. \n";
      }
    } else if (arg_type == "string") {
      value = arg_value;
    }

    parameters[arg_name] = value;
  }
}

fs::Config::Config(Logger& log): log(log) {}

fs::Config::Config(Logger& log, const std::string& path): log(log) {
  load(path);
}

int fs::get_file_count(const std::string& path) {
  using std::filesystem::directory_iterator;
  return std::distance(directory_iterator(path), directory_iterator{});
}