#include "server.hpp"

bool operator<(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->descriptor < second.socket->descriptor;
}

bool operator==(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->descriptor == second.socket->descriptor;
}

Server::Connection::Connection(Socket* socket)
  : socket(socket), status(unauthorized) {};


Server::Server(int port, Storage& storage, Encoder& encoder)
  : socket(Socket(port)), storage(storage), encoder(encoder) {
  if (socket.setSocketOption(SO_REUSEADDR, 1) != 0)
    std::cout << "Socket option setting failed." << std::endl;

  if (socket.bind() != 0)
    std::cout << "Binding failed." << std::endl;

  if (socket.listen(3) != 0)
    std::cout << "Listen failed." << std::endl;

  std::cout << "Server constructed\n";
}

void Server::acceptConnection() {
  auto* new_socket = new Socket(socket.accept());
  std::cout << "Accepted new connection, FD(" << new_socket->descriptor << ") ";
  std::cout << "ip: " << new_socket->getIpAddress();
  std::cout << ":" << new_socket->getPort() << "\n";
  // new_socket->send("Hello, you have been connected.");
  connections.emplace_back(new_socket);
}

void Server::removeConnection(const Connection& peer) {
  peer.socket->getPeerName();
  std::cout << "Peer disconnected, FD(" << peer.socket->descriptor << ") ";
  std::cout << "ip: " << peer.socket->getIpAddress();
  std::cout << ":" << peer.socket->getPort() << "\n";
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

  if (commandType == "/addfriend") {
    userid_t target;
    ss >> target;
    storage.addFriend(user.user, target);
  } else if (commandType == "/myid") {
    callback.message = "Your ID is: " + std::to_string(user.user);
  } else if (commandType == "/chat") {
    callback.message = "You are in chat with ID: " + std::to_string(storage.getChat(user.user));
  } else if (commandType == "/makechat") {
    chatid_t id = storage.createChat(user.user);
    storage.setUserChat(user.user, id);
    callback.message = "You have successfully created chat with ID: " + std::to_string(id);
  } else if (commandType == "/invite") {
    chatid_t currentChat = storage.getChat(user.user);
    if (!currentChat) {
      callback.message = "You cannot invite to the global chat";
    } else {
      userid_t target;
      ss >> target;
      storage.inviteToChat(user.user, target, currentChat);
      callback.message = "You have invited " + storage.getUserReference(target).getNickname() 
        + " to chat " + std::to_string(currentChat);
    }
  } else if (commandType == "/switchchat") {
    chatid_t id;
    ss >> id;
    int code = storage.setUserChat(user.user, id);
    if (code == -1) {
      callback.message = "No chat found.";
    } else if (code == -2) {
      callback.message = "You are not a member of the chat";
    } else {
      callback.message = "You have succesfully switched the chat";
    }
  } else if (commandType == "/friends") {
    const std::vector<userid_t>& friends = storage.getUserFriends(user.user);
    callback.message = "Your friends are: ";
    for (auto &usr : friends) {
      callback.message += storage.getUserReference(usr).getNickname() + ", ";
    }
  } else if (commandType == "/chats") {
    const std::vector<chatid_t>& userchats = storage.getUserChats(user.user);
    callback.message += "Your available chats are: ";
    for (auto &cht : userchats) {
      callback.message += std::to_string(cht);
    }
  }
  user.socket->send(encoder.encode(callback));
}

void Server::addMessage(Object object, Connection& user) {
  object.author = user.user;
  const User& usr = storage.getUserReference(user.user);
  object.message = "[" + usr.getNickname() + "] " + object.message;
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
