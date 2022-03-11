#include "server.hpp"

Server::Server(int port): socket(Socket(port)) {
  if (socket.setSocketOption(SO_REUSEADDR, 1) != 0) {
    std::cout << "Socket option setting failed." << std::endl;
  }

  if (socket.bind() != 0) {
    std::cout << "Binding failed." << std::endl;
  }

  if (socket.listen(3) != 0) {
    std::cout << "Listen failed." << std::endl;
  }

  std::cout << "Server constructed\n";
}

void Server::loop() {
  while (true) {
    fillSocketSet();
    
    int maxDescriptor = socket.descriptor;

    for (auto &x : sockets) {
      FD_SET(x->descriptor, &readset);
      maxDescriptor = std::max(maxDescriptor, x->descriptor);
    }
  
    int activity = cstd::select(maxDescriptor + 1, &readset, NULL, NULL, NULL);
    if (FD_ISSET(socket.descriptor, &readset)) {
      Socket* new_socket = new Socket(socket.accept());
      std::cout << "Accepted new connection, FD(" << new_socket->descriptor << ") ";
      std::cout << "ip: " << new_socket->getIpAddress() << ":" << new_socket->getPort() << "\n";
      new_socket->send("Hello, you have been connected.");
      sockets.insert(new_socket);
    }

    std::vector<Socket*> disconnected;
    for (auto &sd : sockets) {
      if (FD_ISSET(sd->descriptor, &readset)) {
        char buffer[1024];
        
        int valread = cstd::read(sd->descriptor , buffer, 1024);

        if (valread == 0) {
          sd->getPeerName();
          std::cout << "Peer disconnected, FD(" << sd->descriptor << ") ";
          std::cout << "ip: " << sd->getIpAddress() << ":" << sd->getPort() << "\n";
          sd->~Socket();
          disconnected.push_back(sd);
        } else {
          // LEGACY
          buffer[valread] = '\0';
          std::cout << "Received: " << buffer << ' ' << valread << '\n';
          for (auto &jd : sockets) {
            if (jd == sd) {
              continue;
            }
            std::cout << "Trying to send to " << jd->descriptor << " - ";

            jd->send(buffer, valread);
          }
          
        }
      }
    }
    while (!disconnected.empty()) {
      sockets.erase(disconnected.back());
      disconnected.pop_back();
    }
  }
}

void Server::fillSocketSet() {
  FD_ZERO(&readset);
  FD_SET(socket.descriptor, &readset);
}