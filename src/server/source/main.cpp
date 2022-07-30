#pragma once

#include "server.hpp"

int main() {
  Logger log = {&std::cout};

  fs::Config config(log);
  config.load("./config/server.cfg");

  StrEncoder encoder;
  SmartStorage storage("./config/storage.cfg", log, encoder);

  Server server(config.get<int>("port"), storage, encoder, log);

  server.loop();
}