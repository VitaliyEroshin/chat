#include "storage.hpp"
#include <string.h>
#include <sstream>
#include <filesystem>

SmartStorage::SmartStorage(const std::string& configPath, Logger& logger)
    : config(logger, configPath),
      users(2, config.get<std::string>("userAuthDataPath")),
      userdata(2, config.get<std::string>("userDataPath")),
      friends(2, config.get<std::string>("friendsDataPath")),
      chats(2, config.get<std::string>("chatsDataPath")),
      availableChats(2, config.get<std::string>("availableChatsDataPath"))
{
  std::filesystem::create_directories(config.get<std::string>("userAuthDataPath"));
  std::filesystem::create_directories(config.get<std::string>("userDataPath"));
  std::filesystem::create_directories(config.get<std::string>("friendsDataPath"));
  std::filesystem::create_directories(config.get<std::string>("chatsDataPath"));
  std::filesystem::create_directories(config.get<std::string>("availableChatsDataPath"));
}

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

int SmartStorage::getUserCount() {
  if (userCount != -1) {
    return userCount;
  }
  std::string path = config.get<std::string>("userDataPath");
  
  int blocksCount = fs::getFileCount(path);
  if (blocksCount == 0) {
    userCount = 0;
    return 0;
  }
  Block& block = userdata[blocksCount - 1];
  std::stringstream ss(block[block.size()]);
  std::string id;
  ss >> id;
  userCount = std::stoi(id);
  return userCount;
}

int SmartStorage::getUserDataBlock(size_t id) {
  return (id - 1) / config.get<int>("userDataBlockSize");
}

int SmartStorage::getUserDataBlockPosition(size_t id) {
  return (id - 1) % config.get<int>("userDataBlockSize");
}

int SmartStorage::addUser(const login_t& login, const password_t& password) {
  int userStatus = getUser(login, password);
  if (userStatus != -1) {
    return -1;
  }

  userid_t id = getUserCount() + 1;
  Block& block = users[getPrefix(login, 1)];
  std::string s = login + " " + password + " " + std::to_string(id);
  block.add(s);
  block.save(config.get<std::string>("userAuthDataPath") + getPrefix(login, 1));

  int blockId = getUserDataBlock(id);
  Block& dataBlock = userdata[blockId];
  s = std::to_string(id) + " " + login;
  dataBlock.add(s);
  ++userCount;
  dataBlock.save(config.get<std::string>("userDataPath") + std::to_string(blockId));
  return id;
}

int SmartStorage::getChatsCount() {
  if (chatCount != -1) {
    return chatCount;
  }
  std::string path = config.get<std::string>("chatsDataPath");
  
  int blocksCount = fs::getFileCount(path);
  if (blocksCount == 0) {
    chatCount = 0;
    return 0;
  }
  
  chatCount = blocksCount;
  return chatCount;
}

bool SmartStorage::isMember(Block& block, userid_t member) {
  return block[0].find(" " + std::to_string(member) + " ") != std::string::npos;
}

bool SmartStorage::isMember(chatid_t chat, userid_t member) {
  Block& block = chats[std::to_string(chat)];
  return isMember(block, member);
}

int SmartStorage::createChat(userid_t creator) {
  chatid_t id = getChatsCount() + 1;
  Block& block = chats[std::to_string(id)];
  block[0] = " " + std::to_string(creator) + " ";
  block.save();
  addAvailableChat(creator, id);
  ++chatCount;
  return 0;
}

void SmartStorage::addAvailableChat(userid_t id, chatid_t chat) {
  Block& block = availableChats[getUserDataBlock(id)];
  block[getUserDataBlockPosition(id)] += std::to_string(chat) + " ";
  block.save();
}

int SmartStorage::inviteToChat(userid_t selfId, userid_t target, chatid_t chat) {
  if (target == selfId || target > userCount) {
    return -4;
  }

  if (chat > chatCount) {
    return -1;
  }

  Block& block = chats[std::to_string(chat)];
  if (!isMember(block, selfId)) {
    return -2;
  }

  if (!isMember(block, target)) {
    addAvailableChat(target, chat);
    block[1] += std::to_string(target) + " ";
    block.save();
  }

  return 0;
}

chatid_t SmartStorage::getChat(userid_t selfId) {
  if (!currentChat.count(selfId)) {
    currentChat[selfId] = 0;
    chatListeners[0].insert(selfId);
  }
  return currentChat[selfId];
}

int SmartStorage::setUserChat(userid_t id, chatid_t chat) {
  if (chat < 0 || chat > chatCount) {
    return -1;
  };

  if (!isMember(chat, id)) {
    return -2;
  };

  if (currentChat.count(id)) {
    chatListeners[currentChat[id]].erase(id);
  }

  currentChat[id] = chat;
  chatListeners[chat].insert(id);
  return 0;
}

std::vector<chatid_t> SmartStorage::getUserChats(userid_t id) {
  Block& block = availableChats[getUserDataBlock(id)];
  std::stringstream ss(block[getUserDataBlockPosition(id)]);
  std::string chatId;

  std::vector<chatid_t> chatlist;
  while (ss >> chatId) {
    chatlist.push_back(std::stoi(chatId));
  }
  return std::move(chatlist);
}
  
std::vector<userid_t> SmartStorage::getUserFriends(userid_t id) {
  Block& block = friends[getUserDataBlock(id)];
  std::stringstream ss(block[getUserDataBlockPosition(id)]);
  std::string friendId;

  std::vector<userid_t> friendlist;
  while (ss >> friendId) {
    friendlist.push_back(std::stoi(friendId));
  }
  return std::move(friendlist);
}

int SmartStorage::addFriend(userid_t selfId, userid_t target) {
  if (target == selfId || target > userCount) {
    return -1;
  }

  Block& block = friends[getUserDataBlock(selfId)];
  std::string& s = block[getUserDataBlockPosition(selfId)];
  s += std::to_string(target) + " ";
  block.save();
  return 0;
}

std::string SmartStorage::getUserNickname(userid_t id) {
  Block& block = userdata[getUserDataBlock(id)];
  std::stringstream ss(block[getUserDataBlockPosition(id)]);
  std::string value;
  ss >> value;
  ss >> value;
  return value;
}