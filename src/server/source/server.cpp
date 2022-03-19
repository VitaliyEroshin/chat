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
  // new_socket->send("Hello, you have been connected.");
  connections.push_back(Connection(new_socket));
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
  
  select(maxDescriptor + 1, &readset, nullptr, nullptr, nullptr);
}

[[noreturn]] void Server::loop() {
  char buffer[1025];
  while (true) {
    selectDescriptor();

    if (FD_ISSET(socket.descriptor, &readset)) {
      acceptConnection();
    }

    for (auto it = connections.begin(); it != connections.end();){
      auto current = it++;
      Connection& peer = *current;
      
      if (FD_ISSET(peer.socket->descriptor, &readset)) {
        std::string query = peer.socket->read();
        if (query.empty()) {
          removeConnection(peer);
          connections.erase(current);
        } else {
          parseQuery(query, peer);
        }
      }
    }
  }
}

void Server::parseQuery(const std::string& query, Connection& user) {
  Object obj = encoder.decode(query);
  std::cout << "Received message from " << user.socket->descriptor << '\n';
  std::cout << "  Message: " << query << '\n';
  
  if (obj.type == Object::Type::text) {
    for (auto &other : connections) {
      if (other == user) {
        continue;
      }
          
      std::cout << "Trying to send to " << other.socket->descriptor << " - ";

      other.socket->send(encoder.encode(obj));
    }
  } else if (obj.type == Object::Type::loginAttempt) {
    parseAuthData(obj.message, user);
  }
}

void Server::parseAuthData(const std::string& query, Connection& user) {
  int ptr;
  std::string login;
  std::string password;

  for (ptr = 0; ptr < query.size() && query[ptr] != 1; ++ptr) {
    login.push_back(query[ptr]);
  }

  for (ptr = ptr + 1; ptr < query.size(); ++ptr) {
    password.push_back(query[ptr]);
  }

  Object obj;
  obj.type = Object::Type::returnCode;

  int code = storage.getUser(login, password);
  
  if (code == -2) {
      // wrong password
      obj.ret = 2;
      user.socket->send(encoder.encode(obj));
      return;
  }
  if (code == -1) {
    storage.addUser(login, password);
    code = storage.getUser(login, password);
    obj.ret = 1;
    user.socket->send(encoder.encode(obj));
  } else {
    obj.ret = 0;
    user.socket->send(encoder.encode(obj));
  }
  user.status = Server::Connection::Status::inmenu;
  user.user = code;
}