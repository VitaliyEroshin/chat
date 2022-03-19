#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <vector>
#include <list>
#include "ui.hpp"
#include "socket.hpp"
#include "encoder.hpp"

struct ObjectTree {
  std::list<Object> objects;
  std::list<Object>::iterator head;

  void insert(const std::string& text);
};

class Client {
private:
  bool setAddress(std::string ip, int port);
  
public:
  enum Status {
    offline, online, connecting, authentification, failed
  };

  Client::Status status;
  Socket socket;
  UserInterface ui;
  Encoder encoder;
  ObjectTree data;

  Client();
  
  void setupAddress();
  int connectToHost();
  int auth();
  void initializeGUI();
  void refreshMessages();

  void sendText(const std::string& text);
};