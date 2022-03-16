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
  if (socket.setSocketOption(SO_REUSEADDR, 1) != 0)
    std::cout << "Socket option setting failed." << std::endl;

  if (socket.bind() != 0)
    std::cout << "Binding failed." << std::endl;

  if (socket.listen(3) != 0)
    std::cout << "Listen failed." << std::endl;

  std::cout << "Server constructed\n";
}

void Server::acceptConnection() {
  Socket* new_socket = new Socket(socket.accept());
  std::cout << "Accepted new connection, FD(" << new_socket->descriptor << ") ";
  std::cout << "ip: " << new_socket->getIpAddress();
  std::cout << ":" << new_socket->getPort() << "\n";
  new_socket->send("Hello, you have been connected.");
  connections.insert(Connection(new_socket));
}

void Server::removeConnection(const Connection& peer) {
  peer.socket->getPeerName();
  std::cout << "Peer disconnected, FD(" << peer.socket->descriptor << ") ";
  std::cout << "ip: " << peer.socket->getIpAddress();
  std::cout << ":" << peer.socket->getPort() << "\n";
  peer.socket->~Socket();
}

void Server::selectDescriptor() {
  FD_ZERO(&readset);
  FD_SET(socket.descriptor, &readset);
    
  int maxDescriptor = socket.descriptor;

  for (auto &x : connections) {
      FD_SET(x.socket->descriptor, &readset);
      maxDescriptor = std::max(maxDescriptor, x.socket->descriptor);
  }
  
  select(maxDescriptor + 1, &readset, NULL, NULL, NULL);
}

void Server::loop() {
  char buffer[1025];
  while (true) {
    selectDescriptor();

    if (FD_ISSET(socket.descriptor, &readset)) {
      acceptConnection();
    }

    std::vector<Connection> disconnected;
    for (auto& peer : connections) {
      if (FD_ISSET(peer.socket->descriptor, &readset)) {
        int readValue = cstd::read(peer.socket->descriptor, buffer, 1024);

        if (readValue == 0) {
          removeConnection(peer);
          disconnected.push_back(peer);
        } else {
          parseQuery(buffer, readValue, peer);
        }
      }
    }
    
    while (!disconnected.empty()) {
      connections.erase(disconnected.back());
      disconnected.pop_back();
    }
  }
}

void Server::parseQuery(char* buffer, int valread, const Connection& user) {
  std::string query;
  for (size_t i = 0; i < valread; ++i) {
    query.push_back(buffer[i]);
  }

  Object obj = encoder.decode(query);
  std::cout << "Received message from " << user.socket->descriptor << '\n';
  std::cout << "  Message: " << obj.message << '\n';
  
  if (obj.type == Object::Type::text) {
    for (auto &other : connections) {
      if (other == user) {
        continue;
      }
          
      std::cout << "Trying to send to " << other.socket->descriptor << " - ";

      other.socket->send(encoder.encode(obj));
    }
  }
}