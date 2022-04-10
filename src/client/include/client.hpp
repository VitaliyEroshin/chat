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
#include "filesystem.hpp"
#include "logger.hpp"

struct ObjectTree {
  std::list<Object> objects;
  std::list<Object>::iterator head;

  void insert(const std::string& text);
  void insert(const Object& obj);
  ObjectTree();
};

class Client {
private:
  bool setAddress(std::string ip, int port);

  enum Status {
    offline, online, connecting, authentification, failed
  };

  Client::Status status;
  Socket socket;
  UserInterface ui;
  Encoder& encoder;
  ObjectTree data;
  fs::Config& config;
  
  void setupAddress();
  int connectToHost();
  int auth();
  void initializeGUI();
  void refreshMessages();
  void sendText(const std::string& text);
  void sendCommand(const std::string& text);

  int connect();
  void showBackground(std::atomic<bool>& connecting);
  void listen();

  void parseMessage(const std::string& message);
  
  void readServer(std::atomic<bool>& update, std::atomic<bool>& run);
  void readUserInput(std::atomic<bool>& update, std::atomic<bool>& run);
  void refreshOutput(std::atomic<bool>& update, std::atomic<bool>& run);

  void scrollup();
  void scrolldown();

  void drawChatPointer();
  void allocateChatSpace();
  void deallocateChatSpace();

  size_t chatspace = 1;

  std::atomic<bool> run;
  std::atomic<bool> update;

  friend UserInterface;
public:
  explicit Client(Encoder& encoder, fs::Config& config);
  int session();
};