#include "udp-server.hpp"

UdpServer::UdpServer(int port, Storage& storage, Encoder& encoder, Logger& log)
    : socket(UdpSocket(port))
    , storage(storage)
    , encoder(encoder)
    , log(log)
{
  if (socket.bind() != 0) {
    log << "Binding failed." << std::endl;
  }

  log << "Server is ready on " << socket.get_ip_address() << std::endl;
  init_handlers();
  log << "Command handlers ready" << std::endl;
}

[[noreturn]] void UdpServer::loop() {
  while (true) {
    auto [query, address] = socket.read();
    if (query == "ping") {
      // new connection
      log << "Received ping, pinging back..." << std::endl;
      socket.send("ping", address);
    } else if (query == "") {
      socket.send("sorry", address);
    } else {
        parse_query(query, address);
    }
  }
}

UdpServer::Connection::Connection(Address address): address(address)
{}

void UdpServer::parse_query(std::string query, Address address) {
  auto task = [this](const std::string& query, Address& address) {
    Object object = encoder.decode(query);
    if (object.type == Object::Type::text) {
      add_message(object, address);
      return;
    }
    if (object.type == Object::Type::loginAttempt) {
      parse_auth_data(object, address);
      return;
    }
    if (object.type == Object::Type::command) {
      parse_command(object, address);
      return;
    }
  };
  task(query, address);
}
std::pair<std::string, std::string> split_auth_data(const std::string& content);

void UdpServer::parse_auth_data(const Object &object, Address &address) {
  auto [login, password] = split_auth_data(object.content);

  auto& user = get_connection_reference(address);

  Object callback;
  callback.type = Object::Type::returnCode;

  const int
          kOk = 0,
          kSignedUp = 0,
          kWrongPassword = 2;

  int result = storage.get_user(login, password);

  switch (result) {
    case -2:
      callback.set_return_code(kWrongPassword);
      break;

    case -1:
      callback.set_return_code(kSignedUp);
      user.user = storage.add_user(login, password);
      break;

    default:
      callback.set_return_code(kOk);
      user.user = result;
  }

  socket.send(encoder.encode(callback), address);
}

void UdpServer::parse_command(const Object& object, Address& address) {
  auto& user = get_connection_reference(address);
  std::stringstream ss;
  ss << object.content;

  std::string command_type;
  ss >> command_type;
  log << "Accepted command " << command_type << std::endl;

  Object callback;
  callback.type = Object::Type::text;
  callback.set_id((object.has_id() ? object.id : 0));

  const int kCommandCallback = 4;
  callback.set_return_code(kCommandCallback);
  auto sendfn = [&user, this](const Object& callback) {
    socket.send(encoder.encode(callback), user.address);
  };

  if (handlers.count(command_type)) {
    auto task = handlers[command_type];
    task(ss, static_cast<ConnectionBase &>(user), object, sendfn, storage);
  } else {
    log << "Unknown command, sorry" << std::endl;
  }
}

void UdpServer::add_message(Object& object, Address& address) {
  auto& user = get_connection_reference(address);
  std::stringstream ss;
  add_message_handler(object, user, ss);
}

UdpServer::Connection& UdpServer::get_connection_reference(Address& address) {
  std::string str_address = socket.get_ip_address(address)
                            + std::to_string(socket.get_port(address));

  if (!ip_to_connection.count(str_address)) {
    ip_to_connection.insert({str_address, Connection(address)});
  }
  return ip_to_connection[str_address];
}

void UdpServer::init_handlers() {
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
void UdpServer::add_handler(const std::string& command, handler_t handler) {
  handlers.insert({command, handler});
}

void UdpServer::add_message_handler(Object& object, Connection& user, std::stringstream& ss) {
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

  for (auto &[ip, other] : ip_to_connection) {
    if (storage.get_chat(other.user) != storage.get_chat(user.user)) {
      continue;
    }

    socket.send(encoder.encode(object), other.address);
  }
}