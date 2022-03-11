#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <vector>
#include "ui.hpp"
#include "socket.hpp"

class Client {
private:
  bool setAddress(std::string ip, int port);
  
public:
  enum Status {
    offline,
    online
  };

  Client::Status status;
  Socket socket;
  UserInterface ui;

  Client();
  
  int connect();
  void readUserInput(std::atomic<bool>& run);
  void readServer(std::atomic<bool>& run);
};