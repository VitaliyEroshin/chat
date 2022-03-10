#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <vector>
#include "socket.hpp"

class Client {
public:
  enum Status {
    offline,
    online
  };
  Client::Status status;
  Socket socket;

  Client();
  bool setAddress(std::string ip, int port);

};