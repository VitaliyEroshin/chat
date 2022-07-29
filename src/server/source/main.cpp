#include "server.hpp"
#include "udp-server.hpp"

#define UDP

#ifdef UDP
using ServerT = UdpServer;
#else
using ServerT = Server;
#endif


int main() {
  Logger log = {&std::cout};

  fs::Config config(log);
  config.load("./config/server.cfg");

  StrEncoder encoder;
  SmartStorage storage("./config/storage.cfg", log, encoder);

  ServerT server(config.get<int>("port"), storage, encoder, log);

  server.loop();
}