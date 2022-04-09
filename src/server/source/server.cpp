#include "server.hpp"

bool operator<(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->descriptor < second.socket->descriptor;
}

bool operator==(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->descriptor == second.socket->descriptor;
}

Server::Connection::Connection(Socket* socket)
  : socket(socket), status(unauthorized) {};


Server::Server(int port, Storage& storage, Encoder& encoder, Logger& log)
    : socket(Socket(port)), 
      storage(storage), 
      encoder(encoder),
      log(log)
{
  if (socket.setSocketOption(SO_REUSEADDR, 1) != 0)
    log << "Socket option setting failed." << std::endl;

  if (socket.bind() != 0)
    log << "Binding failed." << std::endl;

  if (socket.listen(3) != 0)
    log << "Listen failed." << std::endl;

  log << "Server constructed\n";
  initHandlers();
}

void Server::acceptConnection() {
  auto* new_socket = new Socket(socket.accept());
  log << "Accepted new connection, FD(" << new_socket->descriptor << ") ";
  log << "ip: " << new_socket->getIpAddress();
  log << ":" << new_socket->getPort() << "\n";
  connections.emplace_back(new_socket);
}

void Server::removeConnection(const Connection& peer) {
  peer.socket->getPeerName();
  log << "Peer disconnected, FD(" << peer.socket->descriptor << ") ";
  log << "ip: " << peer.socket->getIpAddress();
  log << ":" << peer.socket->getPort() << "\n";
  peer.socket->~Socket();
}

void Server::selectDescriptor() {
  readset.clear();
  readset.set(socket.descriptor);
    
  int maxDescriptor = socket.descriptor;

  for (auto &x : connections) {
      readset.set(x.socket->descriptor);
      maxDescriptor = std::max(maxDescriptor, x.socket->descriptor);
  }

  select(maxDescriptor + 1, readset.reference(), nullptr, nullptr, nullptr);
}

[[noreturn]] void Server::loop() {
  while (true) {
    selectDescriptor();

    if (readset.count(socket.descriptor)) {
      acceptConnection();
    }

    for (auto it = connections.begin(); it != connections.end();) {
      auto current = it++;
      Connection& peer = *current;

      if (readset.count(peer.socket->descriptor)) {
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
  if (obj.type == Object::Type::text) {
    addMessage(obj, user);
    return;
  }
  if (obj.type == Object::Type::loginAttempt) {
    parseAuthData(obj, user);
    return;
  }
  if (obj.type == Object::Type::command) {
    parseCommand(obj, user);
    return;
  }
}

void Server::parseAuthData(const Object& object, Connection& user) {
  int ptr;
  std::string login;
  std::string password;

  for (ptr = 0; ptr < object.message.size() && object.message[ptr] != 1; ++ptr) {
    login.push_back(object.message[ptr]);
  }

  for (ptr = ptr + 1; ptr < object.message.size(); ++ptr) {
    password.push_back(object.message[ptr]);
  }

  Object callback;
  callback.type = Object::Type::returnCode;

  int code = storage.getUser(login, password);
  
  if (code == -2) {
    callback.ret = 2;
    user.socket->send(encoder.encode(callback));
    return;
  }
  if (code == -1) {
    storage.addUser(login, password);
    code = storage.getUser(login, password);
    callback.ret = 1;
    user.socket->send(encoder.encode(callback));
  } else {
    callback.ret = 0;
    user.socket->send(encoder.encode(callback));
  }
  user.status = Server::Connection::Status::inmenu;
  user.user = code;
}

void Server::parseCommand(const Object& object, Connection& user) {
  std::stringstream ss;
  ss << object.message;

  std::string commandType;
  ss >> commandType;

  Object callback;
  callback.type = Object::Type::text;
  callback.id = 0;

  if (handlers.count(commandType)) {
    handlers[commandType](callback, user, ss);
  }

  user.socket->send(encoder.encode(callback));
}

void Server::addMessage(Object object, Connection& user) {
  object.author = user.user;
  object.message = "[" + storage.getUserNickname(user.user) + "] " + object.message;
  for (auto &other : connections) {
    if (other == user) {
      continue;
    }
    if (storage.getChat(other.user) != storage.getChat(user.user)) {
      continue;
    }

    other.socket->send(encoder.encode(object));
  }
}
