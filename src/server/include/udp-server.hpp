#pragma once

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <unordered_set>
#include <sstream>
#include <functional>
#include "udp-socket.hpp"
#include "storage.hpp"
#include "filesystem.hpp"
#include "logger.hpp"

class UdpServer {
  struct Connection {
    Connection();

    enum Status {
      unauthorized,
      inmenu,
      inchat,
      inprofile
    };

    Address address;
    userid_t user{};
    chatid_t chat{};
    Status status;

    explicit Connection(Address address);
  };

  UdpSocket socket;
  Storage& storage;
  Encoder& encoder;
  Logger& log;

  std::unordered_map<std::string, Connection> ip_to_connection;
  std::map<std::string, std::function<void(Object&, Connection&, std::stringstream&)>> handlers;

public:
  explicit UdpServer(int port, Storage& storage, Encoder& encoder, Logger& log);

  [[noreturn]] void loop();

  void parse_query(std::string query, Address address);
  void parse_auth_data(const Object& object, Address& address);
  void parse_command(const Object& object, Address& address);
  void add_message(Object& object, Address& address);
  Connection& get_connection_reference(Address& address);
  ~UdpServer() = default;

  void init_handlers();

  template<typename Handler>
  void add_handler(const std::string& command, Handler handler);


  void addFriendHandler(Object& callback, Connection& user, std::stringstream& ss);
  void getSelfIdHandler(Object& callback, Connection& user, std::stringstream& ss);
  void getChatIdHandler(Object& callback, Connection& user, std::stringstream& ss);
  void makeChatHandler(Object& callback, Connection& user, std::stringstream& ss);
  void inviteToChatHandler(Object& callback, Connection& user, std::stringstream& ss);
  void switchChatHandler(Object& callback, Connection& user, std::stringstream& ss);
  void getFriendsHandler(Object& callback, Connection& user, std::stringstream& ss);
  void getChatsHandler(Object& callback, Connection& user, std::stringstream& ss);
  void getHelpHandler(Object& callback, Connection& user, std::stringstream& ss);
  void getAboutHandler(Object& callback, Connection& user, std::stringstream& ss);

  void add_message_handler(Object& object, Connection& user, std::stringstream& ss);
  void scrollUpHandler(Object& object, Connection& user, std::stringstream& ss);
  void scrollDownHandler(Object& object, Connection& user, std::stringstream& ss);
};