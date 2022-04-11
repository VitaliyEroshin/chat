#include "client.hpp"

int main() {
  Logger log = {&std::cerr};
  StrEncoder encoder;
  
  fs::Config config(log);
  config.load("./config/client.cfg");

  Client client(encoder, config);
  //client.test();
  client.session();
}