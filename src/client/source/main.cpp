#include "client.hpp"
#include "ui.hpp"

int session() {
  Client client;
  client.ui.initWindow();
  int rcode = client.connect();
  if (rcode < 0) {
    std::cout << "Connection failed. (" << rcode << ")\n\r";
    return 1;
  }
  std::cout << "Connected";

  return 0;
}

int main() {
  system("stty raw");
  session();
  
  system("stty cooked");
}