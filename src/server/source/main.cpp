#include <iostream>
#include "server.hpp"

int main() {
  StrEncoder encoder;
  Server server(8888, encoder);
  server.loop();
}