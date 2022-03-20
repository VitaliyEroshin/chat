#include "server.hpp"

int main() {
  RAMStorage storage;
  Server server(8888, storage);
  server.loop();
}