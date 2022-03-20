#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <list>
#include "socket.hpp"
#include "encoder.hpp"
#include "ui.hpp"

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
  Encoder& encoder;
  ObjectTree data;

  explicit Client(Encoder& encoder);
  
  void setupAddress();
  int connectToHost();
  int auth();
  void initializeGUI();
  void refreshMessages();
  void sendText(const std::string& text);

  int session();
  int connect();
  void showBackground(std::atomic<bool>& connecting);
  void listen();

  void readServer(std::atomic<bool>& run);
  void readUserInput(std::atomic<bool>& run);
};