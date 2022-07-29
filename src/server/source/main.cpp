#include "server.hpp"
#include "udp-server.hpp"
void print(Object& obj) {
  int attr = obj.attributes;
  for (int i = 0; i < 8; ++i) {
    std::cout << attr % 2;
    attr /= 2;
  }
  std::cout << "|" <<  obj.content << "\n";
}

int main() {
  Logger log = {&std::cout};

  fs::Config config(log);
  config.load("./config/server.cfg");

  StrEncoder encoder;
  SmartStorage storage("./config/storage.cfg", log, encoder);

  Server server(config.get<int>("port"), storage, encoder, log);

  server.loop();
}