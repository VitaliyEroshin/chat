#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <list>
#include <algorithm>

#include "socket.hpp"
#include "encoder.hpp"
#include "ui.hpp"
#include "filesystem.hpp"
#include "logger.hpp"

struct ObjectTree {
  std::list<Object> objects;
  std::list<Object>::iterator head;

  void insert(const Object& obj);
  void clear();
  bool isMessage(std::list<Object>::iterator message) const;
  int backId() const;
  int frontId() const;
  void pushFront(Object object);
  void pushBack(Object object);
  void propagateIdFromBack();

  ObjectTree();
  ~ObjectTree() = default;
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
  Logger& log;
  
  std::pair<std::string, std::string> askAddress();

  void showAddressHint();
  void setupAddress();
  int connectToHost();
  
  std::pair<std::string, std::string> askAuthData();
  Object makeAuthAttempt(const std::string& username, const std::string& password);
  int printAuthResult(int code);
  int auth();
  
  void initializeGUI();
  bool printMessage(size_t& space, size_t width, const std::string& message);
  void refreshMessages();
  void sendText(const std::string& text);
  void sendCommand(const std::string& text);

  void delay(const std::string& label);
  void showConnectionVerdict(const std::string& verdict);
  int connect();
  void showBackground(std::atomic<bool>& connecting);
  void listen();

  void parseTextObject(Object object);
  void readServer();

  void quit();
  void parseInputCommand(const std::string& command);
  void readUserInput();
  void refreshOutput();

  void scrollup();
  void scrolldown();

  void drawChatPointer();
  void allocateChatSpace();
  void deallocateChatSpace();

  size_t chatspace = 1;
  size_t cachesize = 5;
  size_t commandsContained = 0;

  std::atomic<bool> run;
  std::atomic<bool> update;

  friend UserInterface;
public:
  explicit Client(Encoder& encoder, fs::Config& config, Logger& logger);
  ~Client() = default;
  int session();
};
