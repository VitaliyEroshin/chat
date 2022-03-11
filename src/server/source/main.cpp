#include <iostream>
#include "server.hpp"

int main() {
  Server server(8888);
  server.loop();
}