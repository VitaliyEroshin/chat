#include "server.hpp"

int main() {
  Logger log = {&std::cout};

  fs::Config config(log);
  config.load("./config/server.cfg");

  RAMStorage storage;
  StrEncoder encoder;

  Server server(config.get<int>("port"), storage, encoder, log);

  server.loop();
}