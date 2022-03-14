#include "server.hpp"

bool operator<(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->descriptor < second.socket->descriptor;
}

bool operator==(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->descriptor == second.socket->descriptor;
}

Server::Connection::Connection(Socket* socket)
  : socket(socket), status(unauthorized) {};


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

    for (auto &x : connections) {
      FD_SET(x.socket->descriptor, &readset);
      maxDescriptor = std::max(maxDescriptor, x.socket->descriptor);
    }
  
    int activity = select(maxDescriptor + 1, &readset, NULL, NULL, NULL);
    if (FD_ISSET(socket.descriptor, &readset)) {
      Socket* new_socket = new Socket(socket.accept());
      std::cout << "Accepted new connection, FD(" << new_socket->descriptor << ") ";
      std::cout << "ip: " << new_socket->getIpAddress() << ":" << new_socket->getPort() << "\n";
      new_socket->send("Hello, you have been connected.");
      connections.insert(Connection(new_socket));
    }
    std::vector<Connection> disconnected;
    for (auto &peer : connections) {
      if (FD_ISSET(peer.socket->descriptor, &readset)) {
        char buffer[1025];
        
        int valread = cstd::read(peer.socket->descriptor , buffer, 1024);

        if (valread == 0) {
          peer.socket->getPeerName();
          std::cout << "Peer disconnected, FD(" << peer.socket->descriptor << ") ";
          std::cout << "ip: " << peer.socket->getIpAddress() << ":" << peer.socket->getPort() << "\n";
          peer.socket->~Socket();
          disconnected.push_back(peer.socket);
        } else {
          buffer[valread] = '\0';
          std::cout << "Received: " << buffer << ' ' << valread << '\n';
          for (auto &other : connections) {
            if (other == peer) {
              continue;
            }
          
            std::cout << "Trying to send to " << other.socket->descriptor << " - ";

            other.socket->send(buffer, valread);
          }
        }
      }
    }
    while (!disconnected.empty()) {
      connections.erase(disconnected.back());
      disconnected.pop_back();
    }
  }
}

void Server::fillSocketSet() {
  FD_ZERO(&readset);
  FD_SET(socket.descriptor, &readset);
}