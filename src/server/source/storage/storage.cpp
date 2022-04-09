#include "storage.hpp"
#include <string.h>
#include <sstream>

SmartStorage::SmartStorage(const std::string& configPath, Logger& logger)
    : config(logger, configPath),
      users(2, config.get<std::string>("userAuthDataPath")),
      userdata(2, config.get<std::string>("userDataPath"))
{}

bool isPrefix(const std::string& s, const std::string& prefix) {
  return s.substr(0, prefix.size()) == prefix;
}

std::string getPrefix(const std::string& s, size_t length) {
  std::string prefix;
  for (size_t i = 0; i < length; ++i) {
    prefix.push_back(s.size() > i ? s[i] : '#');
  }
  return prefix;
}

int SmartStorage::getUser(const login_t& login, const password_t& password) {
  Block& block = users[getPrefix(login, 1)];
  
  for (size_t i = 0; i < block.size(); ++i) {
    if (!isPrefix(block[i], login + " ")) {
      continue;
    }
    std::stringstream ss(block[i]);
    std::string value;
    ss >> value;
    ss >> value;
    if (password != value) {
      return -2;
    }
    ss >> value;
    return std::stoi(value);
  }
  return -1;
}

int SmartStorage::addUser(const login_t& login, const password_t& password) {
  int userStatus = getUser(login, password);
  if (userStatus != -1) {
    return -1;
  }

  userid_t id = 1337;
  Block& block = users[getPrefix(login, 1)];
  std::string s = login + " " + password + " " + std::to_string(id);
  block.add(s);
  block.save(config.get<std::string>("userAuthDataPath") + getPrefix(login, 1));
  return id;
}

const User& SmartStorage::getUserReference(userid_t id) {

}

int SmartStorage::createChat(userid_t creator) {

}

int SmartStorage::inviteToChat(userid_t selfId, userid_t target, chatid_t chat) {

}

chatid_t SmartStorage::getChat(userid_t selfId) {

}

int SmartStorage::setUserChat(userid_t id, chatid_t chat) {

}
  
const std::vector<userid_t>& SmartStorage::getUserFriends(userid_t id) {

}

const std::vector<chatid_t>& SmartStorage::getUserChats(userid_t id) {

}

int SmartStorage::addFriend(userid_t selfId, userid_t target) {

}