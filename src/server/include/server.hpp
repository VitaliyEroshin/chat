#pragma once
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>
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
    userid_t user;
    chatid_t chat;
    Status status;

    Connection(Socket* socket);
  };

  Storage storage;
  Encoder encoder;
  friend bool operator<(const Connection& first, const Connection& second);
  friend bool operator==(const Connection& first, const Connection& second);

public:
  Server(int port);
  void loop();
  ~Server() = default;

  Socket socket;

private:
  std::set<Connection> connections;
    
  fd_set readset;
  void fillSocketSet();
  void parseQuery(char* buffer, int valread, const Connection& user);
};