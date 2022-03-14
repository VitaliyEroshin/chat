#include "client.hpp"

Client::Client(): status(Status::offline), socket(Socket(8888)), ui(UserInterface())  {
  std::cout << "Client constructed" << "\n\r";
}

bool Client::setAddress(std::string ip, int port) {
  socket.setAddress(port);
  if (ip == "localhost") {
    ip = "127.0.0.1";
  }
  return cstd::inet_pton(AF_INET, ip.c_str(), &(socket.address.sin_addr));
}

int Client::connect() {
  std::string ip = ui.input("Enter ip address: ");
  int port;
  try {
    port = std::stoi(ui.input("Enter the port: "));
  } catch (...) {

  }

  while (!setAddress(ip, port)) {
    ui.clearPreviousLine();
    ui.clearPreviousLine();
    ui.clearPreviousLine();
    std::cout << "IP or port invalid. Please, try again." << "\n\r";
    ip = ui.input("Enter ip address again: ");
    
    try {
      port = std::stoi(ui.input("Enter the port: "));
    } catch (...) {
      
    }
  }

  return cstd::connect(socket.descriptor, (cstd::sockaddr*)&(socket.address), sizeof(socket.address));
}

void Client::sendText(const std::string& text) {
  Object obj;
  obj.message = text;
  obj.type = Object::Type::text;
  obj.id = 1555;
  std::string en = encoder.encode(obj);
  socket.send(en);
}