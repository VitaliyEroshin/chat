#include "server.hpp"

bool operator<(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->get_descriptor() < second.socket->get_descriptor();
}

bool operator==(const Server::Connection& first, const Server::Connection& second) {
  return first.socket->get_descriptor() == second.socket->get_descriptor();
}

Server::Connection::Connection(Socket* socket)
  : socket(socket) {};


Server::Server(int port, Storage& storage, Encoder& encoder, Logger& log)
    : socket(Socket(port)), 
      storage(storage), 
      encoder(encoder),
      log(log)
{
  if (socket.set_socket_option(SO_REUSEADDR, 1) != 0)
    log << "Socket option setting failed." << std::endl;

  if (socket.bind() != 0)
    log << "Binding failed." << std::endl;

  if (socket.listen(3) != 0)
    log << "Listen failed." << std::endl;

  log << "Server constructed\n";
  init_handlers();
}

void Server::accept_connection() {
  auto* new_socket = new Socket(socket.accept());
  log << "Accepted new connection, FD(" << new_socket->get_descriptor() << ") ";
  log << "ip: " << new_socket->get_ip_address();
  log << ":" << new_socket->get_port() << "\n";
  connections.emplace_back(new_socket);
}

void Server::remove_connection(const Connection& peer) {
  peer.socket->get_peer_name();
  log << "Peer disconnected, FD(" << peer.socket->get_descriptor() << ") ";
  log << "ip: " << peer.socket->get_ip_address();
  log << ":" << peer.socket->get_port() << "\n";
  peer.socket->~Socket();
}

void Server::select_descriptor() {
  readset.clear();
  readset.set(socket.get_descriptor());
    
  int max_descriptor = socket.get_descriptor();

  for (auto &x : connections) {
      readset.set(x.socket->get_descriptor());
    max_descriptor = std::max(max_descriptor, x.socket->get_descriptor());
  }

  readset.select(max_descriptor + 1);
}

[[noreturn]] void Server::loop() {
  while (true) {
    select_descriptor();

    if (readset.count(socket.get_descriptor())) {
      accept_connection();
    }

    for (auto it = connections.begin(); it != connections.end();) {
      auto current = it++;
      Connection& peer = *current;

      if (readset.count(peer.socket->get_descriptor())) {
        std::string query = peer.socket->read();
        if (query.empty()) {
          remove_connection(peer);
          connections.erase(current);
        } else {
          parse_query(query, peer);
        }
      }
    }
  }
}

void Server::parse_query(const std::string& query, Connection& user) {
  Object object = encoder.decode(query);
  if (object.type == Object::Type::text) {
    add_message(object, user);
    return;
  }
  if (object.type == Object::Type::loginAttempt) {
    parse_auth_data(object, user);
    return;
  }
  if (object.type == Object::Type::command) {
    parse_command(object, user);
    return;
  }
}

std::pair<std::string, std::string> split_auth_data(const std::string& content) {
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

void Server::parse_auth_data(const Object& object, Connection& user) {
  auto [login, password] = split_auth_data(object.content);

  Object callback;
  callback.type = Object::Type::returnCode;

  const int 
    kOk = 0,
    kSignedUp = 1,
    kWrongPassword = 2;

  int result = storage.get_user(login, password);
  switch (result) {
    case -2:
      callback.set_return_code(kWrongPassword);
      break;

    case -1:
      callback.set_return_code(kSignedUp);
      user.user = storage.add_user(login, password);
      user.status = Server::Connection::Status::inmenu;
      break;

    default:
      callback.set_return_code(kOk);
      user.user = result;
      user.status = Server::Connection::Status::inmenu;
  }

  user.socket->send(encoder.encode(callback));
}

void Server::parse_command(const Object& object, Connection& user) {
  std::stringstream ss;
  ss << object.content;

  std::string command_type;
  ss >> command_type;

  Object callback;
  callback.type = Object::Type::text;
  callback.set_id((object.has_id() ? object.id : 0));

  const int kCommandCallback = 4;
  callback.set_return_code(kCommandCallback);

  if (handlers.count(command_type)) {
    auto sendfn = [this, user](const Object& callback){
      user.socket->send(encoder.encode(callback));
    };

    auto task = handlers[command_type];
    task(ss, static_cast<ConnectionBase &>(user), object, sendfn, storage);
  }
}

void Server::add_message(Object object, Connection& user) {
  std::stringstream ss;
  add_message_handler(object, user, ss);
}

void Server::add_message_handler(Object& object, Connection& user, std::stringstream& ss) {
  chatid_t chat = storage.get_chat(user.user);

  // TODO
  // auto timestamp = get_timestamp();
  // object.set_timestamp(timestamp);

  object.content = "[" + storage.get_user_nickname(user.user) + "] " + object.content;

  if (chat != 0) {
    log << "Attaching object " << object.info();
    int id = storage.add_message(object, encoder, chat);
    log << "OK" << std::endl;
    object = storage.get_message(id);
  } else {
    object.set_return_code(4);
  }

  for (auto &other : connections) {
    if (storage.get_chat(other.user) != storage.get_chat(user.user)) {
      continue;
    }

    other.socket->send(encoder.encode(object));
  }
}

void Server::init_handlers() {
  add_handler("/addfriend", Handlers::add_friend_handler);
  add_handler("/myid", Handlers::get_self_id_handler);
  add_handler("/chat", Handlers::get_chat_id_handler);
  add_handler("/makechat", Handlers::make_chat_handler);
  add_handler("/invite", Handlers::invite_to_chat_handler);
  add_handler("/switchchat", Handlers::switch_chat_handler);
  add_handler("/friends", Handlers::get_friends_handler);
  add_handler("/chats", Handlers::get_chats_handler);
  add_handler("/help", Handlers::get_help_handler);
  add_handler("/about", Handlers::get_about_handler);
  add_handler("/scrollup", Handlers::scroll_up_handler);
  add_handler("/scrolldown", Handlers::scroll_down_handler);
}

void Server::add_handler(const std::string& command, handler_t handler) {
  handlers.insert({command, handler});
}