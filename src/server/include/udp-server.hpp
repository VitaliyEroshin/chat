#pragma once

#include "handlers.hpp" // handlers, ConnectionBase, types, objects, storage...
#include "udp-socket.hpp"
#include "filesystem.hpp"
#include "logger.hpp"

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <unordered_set>

class UdpServer {
  struct Connection: public ConnectionBase {
    Connection() = default;
    Address address;
    explicit Connection(Address address);
  };

  UdpSocket socket;
  Storage& storage;
  Encoder& encoder;
  Logger& log;

  std::unordered_map<std::string, Connection> ip_to_connection;
  std::unordered_map<std::string, handler_t> handlers;

public:
  explicit UdpServer(int port, Storage& storage, Encoder& encoder, Logger& log);

  [[noreturn]] void loop();

  void parse_query(std::string query, Address address);
  void parse_auth_data(const Object& object, Address& address);
  void parse_command(const Object& object, Address& address);
  void add_message(Object& object, Address& address);
  Connection& get_connection_reference(Address& address);
  ~UdpServer() = default;

  void init_handlers();

  void add_handler(const std::string& command, handler_t handler);
  void add_message_handler(Object& object, Connection& user, std::stringstream& ss);
};