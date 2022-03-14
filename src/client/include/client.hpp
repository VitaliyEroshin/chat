#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <vector>
#include "ui.hpp"
#include "socket.hpp"
#include "encoder.hpp"

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
  Encoder encoder;

  Client();
  
  int connect();
  void sendText(const std::string& text);
};