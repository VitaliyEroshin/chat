#pragma once
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>
#include "socket.hpp"

class Server {
public:
  Server(int port);
  void loop();
  ~Server() = default;

  Socket socket;

private:
  std::set<Socket*> sockets;
    
  fd_set readset;
  void fillSocketSet();
};