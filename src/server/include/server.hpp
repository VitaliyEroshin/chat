#pragma once
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <unordered_set>
#include <sstream>
#include "socket.hpp"
#include "storage.hpp"

class Server {
  struct Connection {
    enum Status {
      unauthorized,
      inmenu,
      inchat,
      inprofile
    };

    Socket* socket;
    userid_t user{};
    chatid_t chat{};
    Status status;

    explicit Connection(Socket* socket);
  };

  Storage& storage;
  Encoder& encoder;

  friend bool operator<(const Connection& first, const Connection& second);
  friend bool operator==(const Connection& first, const Connection& second);

public:
  explicit Server(int port, Storage& storage, Encoder& encoder);

  [[noreturn]] void loop();
  ~Server() = default;

  Socket socket;

private:
  std::list<Connection> connections;

  DescriptorSet readset{};
  void acceptConnection();
  void selectDescriptor();
  static void removeConnection(const Connection& peer);
  void parseQuery(const std::string& query, Connection& user);
  void parseAuthData(const Object& object, Connection& user);
  void parseCommand(const Object& object, Connection& user);
  void addMessage(Object object, Connection& user);
};