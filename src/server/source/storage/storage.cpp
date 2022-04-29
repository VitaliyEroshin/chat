#include "storage.hpp"
#include <string.h>
#include <sstream>
#include <filesystem>

void SmartStorage::addModule(const std::string& alias, size_t lruSize) {
  data.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(alias),
    std::forward_as_tuple(2, config.get<std::string>(alias + "Path"))
  );
}

SmartStorage::SmartStorage(const std::string& configPath, Logger& logger)
    : log(logger),
      config(logger, configPath)
{
  const size_t lruSize = 2;

  std::vector<std::string> modules = {
    "users",
    "userdata",
    "friends",
    "chats",
    "availableChats",
    "messages",
    "messagechatid"
  };

  for (auto& module : modules) {
    addModule(module, lruSize);
  }
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
  Block& block = data["users"][getPrefix(login, 1)];
  
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
  std::string path = config.get<std::string>("userdataPath");
  
  int blocksCount = fs::getFileCount(path);
  if (blocksCount == 0) {
    userCount = 0;
    return 0;
  }
  Block& block = data["userdata"][blocksCount - 1];
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
  Block& block = data["users"][getPrefix(login, 1)];
  std::string s = login + " " + password + " " + std::to_string(id);
  block.add(s);
  block.save(config.get<std::string>("usersPath") + getPrefix(login, 1));

  int blockId = getUserDataBlock(id);
  Block& dataBlock = data["userdata"][blockId];
  s = std::to_string(id) + " " + login;
  dataBlock.add(s);
  ++userCount;
  dataBlock.save(config.get<std::string>("userdataPath") + std::to_string(blockId));
  return id;
}

int SmartStorage::getChatsCount() {
  if (chatCount != -1) {
    return chatCount;
  }
  std::string path = config.get<std::string>("chatsPath");
  
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
  Block& block = data["chats"][std::to_string(chat)];
  return isMember(block, member);
}

int SmartStorage::createChat(userid_t creator) {
  chatid_t id = getChatsCount() + 1;
  Block& block = data["chats"][std::to_string(id)];
  block[0] = " " + std::to_string(creator) + " ";
  block[1] = "0";
  block.save();
  addAvailableChat(creator, id);
  ++chatCount;
  return id;
}

void SmartStorage::addAvailableChat(userid_t id, chatid_t chat) {
  Block& block = data["availableChats"][getUserDataBlock(id)];
  block[getUserDataBlockPosition(id)] += std::to_string(chat) + " ";
  block.block_size = std::max(block.size(), static_cast<size_t>(getUserDataBlockPosition(id) + 1));
  block.save();
}

int SmartStorage::inviteToChat(userid_t selfId, userid_t target, chatid_t chat) {
  if (target == selfId || target > userCount) {
    return -4;
  }

  if (chat > chatCount) {
    return -1;
  }

  Block& block = data["chats"][std::to_string(chat)];
  if (!isMember(block, selfId)) {
    return -2;
  }

  if (!isMember(block, target)) {
    addAvailableChat(target, chat);
    block[0] += std::to_string(target) + " ";
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
  if (chat < 0 || chat > getChatsCount()) {
    return -1;
  };

  if (chat != 0 && !isMember(chat, id)) {
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
  Block& block = data["availableChats"][getUserDataBlock(id)];
  std::stringstream ss(block[getUserDataBlockPosition(id)]);
  std::string chatId;

  std::vector<chatid_t> chatlist;
  while (ss >> chatId) {
    chatlist.push_back(std::stoi(chatId));
  }
  return chatlist;
}
  
std::vector<userid_t> SmartStorage::getUserFriends(userid_t id) {
  Block& block = data["friends"][getUserDataBlock(id)];
  std::stringstream ss(block[getUserDataBlockPosition(id)]);
  std::string friendId;

  std::vector<userid_t> friendlist;
  while (ss >> friendId) {
    friendlist.push_back(std::stoi(friendId));
  }
  return friendlist;
}

bool SmartStorage::isFriend(const std::string& s, userid_t target) {
  return s.find(" " + std::to_string(target) + " ") != std::string::npos;
}

int SmartStorage::addFriend(userid_t selfId, userid_t target) {
  if (target == selfId || target > userCount) {
    return -1;
  }

  Block& block = data["friends"][getUserDataBlock(selfId)];
  std::string& s = block[getUserDataBlockPosition(selfId)];

  if (isFriend(s, target)) {
    return -2;
  }
  if (s.empty()) {
    s.push_back(' ');
  }
  s += std::to_string(target) + " ";
  block.block_size = std::max(block.size(), 
    static_cast<size_t>(getUserDataBlockPosition(selfId)) + 1);

  block.save();
  return 0;
}

std::string SmartStorage::getUserNickname(userid_t id) {
  Block& block = data["userdata"][getUserDataBlock(id)];
  std::stringstream ss(block[getUserDataBlockPosition(id)]);
  std::string value;
  ss >> value;
  ss >> value;
  return value;
}

int SmartStorage::getMessageCount() {
  if (messageCount != -1) {
    return messageCount;
  }

  std::string path = config.get<std::string>("messagesPath");
  
  int blocksCount = fs::getFileCount(path);
  if (blocksCount == 0) {
    messageCount = 0;
    return 0;
  }
  Block& block = data["messages"][blocksCount - 1];
  std::stringstream ss(block[block.size() - 1]);
  std::string id;
  ss >> id;
  messageCount = std::stoi(id);
  return messageCount;
}

int SmartStorage::getMessageBlock(int id) {
  return (id - 1) / config.get<int>("messageBlockSize");
}

int SmartStorage::getMessageBlockPosition(int id) {
  return (id - 1) % config.get<int>("messageBlockSize");
}

void SmartStorage::setMessage(Object object, Encoder& encoder, chatid_t chatid) {
  if (!object.hasId()) {
    object.setId(getMessageCount() + 1);
    ++messageCount;
  }

  Block& block = data["messages"][getMessageBlock(object.id)];
  std::string& s = block[getMessageBlockPosition(object.id)];
  s = encoder.encode(object);
  block.save();

  Block& idblock = data["messagechatid"][getMessageBlock(object.id)];
  std::string& sid = idblock[getMessageBlockPosition(object.id)];
  sid = std::to_string(chatid);
  idblock.save();
}

void SmartStorage::addMessage(Object object, Encoder& encoder, chatid_t chatid) {
  Block& block = data["chats"][std::to_string(chatid)];
  int prev = std::stoi(block[1]);
  
  object.setPrev(prev);
  object.setId(getMessageCount() + 1);
  ++messageCount;
  
  if (prev != 0) {
    Object p = getMessage(prev, encoder);
    p.setNext(object.id);
    setMessage(p, encoder, chatid);
  }
  
  setMessage(object, encoder, chatid);
  block[1] = std::to_string(object.id);
  block.save();
}

Object SmartStorage::getMessage(int id, Encoder& encoder) {
  Block& block = data["messages"][getMessageBlock(id)];
  std::string& s = block[getMessageBlockPosition(id)];
  if (s.empty()) {
    Object object;
    object.setReturnCode(-1);
    return object;
  }
  
  return encoder.decode(s);
}

Object SmartStorage::getLastMessage(Encoder& encoder, chatid_t chatid) {
  Block& block = data["chats"][std::to_string(chatid)];
  int id = std::stoi(block[1]);
  return getMessage(id, encoder);
}

chatid_t SmartStorage::getMessageChatid(int id) {
  Block& block = data["messagechatid"][getMessageBlock(id)];
  std::string& s = block[getMessageBlockPosition(id)];
  if (s.empty()) {
    Object object;
    object.setReturnCode(-1);
    return -1;
  }

  return std::stoi(s);
}