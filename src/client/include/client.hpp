#pragma once

#include <string>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <list>
#include <algorithm>

#ifdef UDP
#include "udp-socket.hpp"
using Socket = UdpSocket;
#else
#include "socket.hpp"
#endif

#include "encoder.hpp"
#include "ui.hpp"
#include "filesystem.hpp"
#include "logger.hpp"

struct ObjectTree {
  std::list<Object> objects;
  std::list<Object>::iterator head;

  void insert(const Object& obj);
  void clear();
  bool is_message(std::list<Object>::iterator message) const;
  int back_id() const;
  int frontId() const;
  void push_front(const Object& object);
  void push_back(const Object& object);
  void propagate_id_from_back();

  ObjectTree();
  ~ObjectTree() = default;
};

class Client {
private:
  bool set_address(std::string ip, int port);

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
  
  std::pair<std::string, std::string> ask_address();

  void show_address_hint();
  void setup_address();
  int connect_to_host();
  
  std::pair<std::string, std::string> ask_auth_data();
  static Object make_auth_attempt(const std::string& username, const std::string& password);
  int print_auth_results(int code);
  int auth();
  
  void init_gui();
  bool print_message(size_t& space, size_t width, const std::string& message);
  void refersh_messages();
  void send_text(const std::string& text);
  void send_command(const std::string& text);

  void delay(const std::string& label);
  void show_connection_verdict(const std::string& verdict);
  int connect();
  void show_background(std::atomic<bool>& connecting);
  void listen();

  void parse_text_object(Object object);
  void read_server();

  void quit();
  void parse_input_command(const std::string& command);
  void read_user_input();
  void refresh_output();

  void get_previous_messages();

  void scrollup();
  void scrolldown();

  void draw_chat_ptr();
  void alloc_chat_space();
  void dealloc_chat_space();

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
