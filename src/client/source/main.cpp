#include "client.hpp"

int main() {
  setlocale(LC_ALL, "en_US.UTF-8");
  Logger log = {&std::cerr};
  StrEncoder encoder;
  
  fs::Config config(log);
  config.load("./config/client.cfg");

  Client client(encoder, config);
  client.session();
}