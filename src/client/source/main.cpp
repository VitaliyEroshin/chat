#include "client.hpp"

int main() {
  setlocale(LC_ALL, "en_US.UTF-8");
  Logger log = {};
  StrEncoder encoder;
  
  fs::Config config(log);
  config.load("./config/client.cfg");

  Client client(encoder, config, log);
  client.session();
}