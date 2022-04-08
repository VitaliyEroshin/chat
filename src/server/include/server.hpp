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

class Server {
  struct Connection {
    enum Status {
      unauthorized,
      inmenu,
      inchat,
      inprofile
    };

    Socket* socket;
    userid_t user{};
    chatid_t chat{};
    Status status;

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
  void acceptConnection();
  void selectDescriptor();
  void removeConnection(const Connection& peer);
  void parseQuery(const std::string& query, Connection& user);
  void parseAuthData(const Object& object, Connection& user);
  void parseCommand(const Object& object, Connection& user);
  void addMessage(Object object, Connection& user);

  void initHandlers();
  
  template<typename Handler>
  void addHandler(const std::string& command, Handler handler);


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
};