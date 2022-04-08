#include "server.hpp"

int main() {
  RAMStorage storage;
  StrEncoder encoder;
  Logger log = {&std::cout};
  Server server(8888, storage, encoder, log);

  server.loop();
}