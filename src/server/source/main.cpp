#include "server.hpp"

int main() {
  RAMStorage storage;
  StrEncoder encoder;
  Server server(8888, storage, encoder);

  server.loop();
}