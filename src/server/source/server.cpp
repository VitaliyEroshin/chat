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
  Object object = encoder.decode(query);
  if (object.type == Object::Type::text) {
    addMessage(object, user);
    return;
  }
  if (object.type == Object::Type::loginAttempt) {
    parseAuthData(object, user);
    return;
  }
  if (object.type == Object::Type::command) {
    parseCommand(object, user);
    return;
  }
}

std::pair<std::string, std::string> splitAuthData(const std::string& content) {
  std::string login, password;
  int ptr;
  for (ptr = 0; ptr < content.size() && content[ptr] != 1; ++ptr) {
    login.push_back(content[ptr]);
  }

  for (ptr = ptr + 1; ptr < content.size(); ++ptr) {
    password.push_back(content[ptr]);
  }
  
  return std::make_pair(login, password);
}

void Server::parseAuthData(const Object& object, Connection& user) {
  auto [login, password] = splitAuthData(object.content);

  Object callback;
  callback.type = Object::Type::returnCode;

  const int 
    kOk = 0,
    kSignedUp = 1,
    kWrongPassword = 2;

  int result = storage.getUser(login, password);
  switch (result) {
    case -2:
      callback.setReturnCode(kWrongPassword);
      break;

    case -1:
      callback.setReturnCode(kSignedUp);
      user.user = storage.addUser(login, password);
      user.status = Server::Connection::Status::inmenu;
      break;

    default:
      callback.setReturnCode(kOk);
      user.user = result;
      user.status = Server::Connection::Status::inmenu;
  }

  user.socket->send(encoder.encode(callback));
}

void Server::parseCommand(const Object& object, Connection& user) {
  std::stringstream ss;
  ss << object.content;

  std::string commandType;
  ss >> commandType;

  Object callback;
  callback.type = Object::Type::text;
  callback.setId(0);

  const int kCommandCallback = 4;
  callback.setReturnCode(kCommandCallback);

  if (handlers.count(commandType)) {
    handlers[commandType](callback, user, ss);
  }

  user.socket->send(encoder.encode(callback));
}

void Server::addMessage(Object object, Connection& user) {
  std::stringstream ss;
  addMessageHandler(object, user, ss);
}
