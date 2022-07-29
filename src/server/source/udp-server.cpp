#include "udp-server.hpp"

UdpServer::UdpServer(int port, Storage& storage, Encoder& encoder, Logger& log)
    : socket(UdpSocket(port))
    , storage(storage)
    , encoder(encoder)
    , log(log)
{
  log << "Descriptor is " <<  socket.get_descriptor() << std::endl;
  if (socket.bind() != 0) {
    log << "Binding failed." << std::endl;
  }

  log << "Server is ready on " << socket.get_ip_address() ;
}

[[noreturn]] void UdpServer::loop() {
  while (true) {
    auto [query, address] = socket.read();
    if (query == "ping") {
      // new connection
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

UdpServer::Connection::Connection() {

}

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
//std::pair<std::string, std::string> split_auth_data(const std::string& content) {
//  std::string login, password;
//  int ptr;
//  for (ptr = 0; ptr < content.size() && content[ptr] != 1; ++ptr) {
//    login.push_back(content[ptr]);
//  }
//
//  for (ptr = ptr + 1; ptr < content.size(); ++ptr) {
//    password.push_back(content[ptr]);
//  }
//
//  return std::make_pair(login, password);
//}

void UdpServer::parse_auth_data(const Object &object, Address &address) {
  auto [login, password] = split_auth_data(object.content);

  auto user = get_connection_reference(address);

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
      break;

    default:
      callback.set_return_code(kOk);
      user.user = result;
  }

  socket.send(encoder.encode(callback), address);
}

void UdpServer::parse_command(const Object& object, Address& address) {
  auto user = get_connection_reference(address);
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
    handlers[command_type](callback, user, ss);
  }

  socket.send(encoder.encode(callback), user.address);
}

void UdpServer::add_message(Object& object, Address& address) {
  auto user = get_connection_reference(address);
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