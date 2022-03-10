#include "client.hpp"

Client::Client(): status(Status::offline) {
  std::cout << "Client constructed" << std::endl;
}

bool Client::setAddress(std::string ip, int port) {
  socket.setAddress(port);
  return cstd::inet_pton(AF_INET, ip.c_str(), &(socket.address.sin_addr));
}