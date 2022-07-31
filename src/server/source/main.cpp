#include "server.hpp"
#include "constants.hpp"

int main() {
  Logger log = {&std::cout};

  fs::Config config(log);
  config.load("./config/server.cfg", ServerConfig::get_default_values());

  StrEncoder encoder;
  SmartStorage storage("./config/storage.cfg", log, encoder);

  Server server(config.get<int>("port"), storage, encoder, log, 4);

  server.loop();
}