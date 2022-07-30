#pragma once
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <unordered_set>
#include <sstream>
#include <functional>
#include "socket.hpp"
#include "storage.hpp"
#include "filesystem.hpp"
#include "logger.hpp"

struct ConnectionBase {
public:
  enum Status {
    unauthorized,
    inmenu,
    inchat,
    inprofile
  };

  userid_t user{};
  chatid_t chat{};
  Status status = unauthorized;
};

class Server {
  struct Connection: public ConnectionBase {
    Socket* socket;
    explicit Connection(Socket* socket);
  };

  Storage& storage;
  Encoder& encoder;
  Logger& log;

  friend bool operator<(const Connection& first, const Connection& second);
  friend bool operator==(const Connection& first, const Connection& second);

public:
  explicit Server(int port, Storage& storage, Encoder& encoder, Logger& log);

  [[noreturn]] void loop();
  ~Server() = default;

  Socket socket;

private:
  std::list<Connection> connections;
  std::map<std::string, std::function<void(Object&, Connection&, std::stringstream&)>> handlers;

  DescriptorSet readset{};
  void accept_connection();
  void select_descriptor();
  void remove_connection(const Connection& peer);
  void parse_query(const std::string& query, Connection& user);
  void parse_auth_data(const Object& object, Connection& user);
  void parse_command(const Object& object, Connection& user);
  void add_message(Object object, Connection& user);

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