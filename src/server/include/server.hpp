#include "socket.hpp"
#include "filesystem.hpp"
#include "logger.hpp"
#include "handlers.hpp" // handlers, ConnectionBase, types, objects, storage...

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <unordered_set>

class Server {
  struct Connection: public ConnectionBase {
    Socket* socket;
    explicit Connection(Socket* socket);
  };

  Storage& storage;
  Encoder& encoder;
  Logger& log;

  friend bool operator<(const Connection& first, const Connection& second);
  friend bool operator==(const Connection& first, const Connection& second);

public:
  explicit Server(int port, Storage& storage, Encoder& encoder, Logger& log);

  [[noreturn]] void loop();
  ~Server() = default;

  Socket socket;

private:
  std::list<Connection> connections;
  std::map<std::string, handler_t> handlers;

  DescriptorSet readset{};
  void accept_connection();
  void select_descriptor();
  void remove_connection(const Connection& peer);
  void parse_query(const std::string& query, Connection& user);
  void parse_auth_data(const Object& object, Connection& user);
  void parse_command(const Object& object, Connection& user);
  void add_message(Object object, Connection& user);

  void init_handlers();
  void add_handler(const std::string& command, handler_t handler);
  void add_message_handler(Object &object, Connection &user, std::stringstream &ss);
};