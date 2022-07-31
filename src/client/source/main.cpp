#include "client.hpp"
#include "constants.hpp" // config default values

int main() {
  setlocale(LC_ALL, "en_US.UTF-8");
  Logger log = {};
  StrEncoder encoder;
  
  fs::Config config(log);
  config.load("./config/client.cfg", ClientConfig::get_default_values());

  Client client(encoder, config, log);
  client.session();
}