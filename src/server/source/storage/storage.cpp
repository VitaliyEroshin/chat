#include "storage.hpp"
#include <string.h>
#include <sstream>
#include <filesystem>

void SmartStorage::add_module(const std::string& alias, size_t lru_size) {
  data.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(alias),
    std::forward_as_tuple(2, config.get<std::string>(alias + "Path"))
  );
}

SmartStorage::SmartStorage(const std::string& cfg_path, Logger& logger, Encoder& encoder)
    : log(logger)
    , config(logger, cfg_path)
    , encoder(encoder)
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
    add_module(module, lruSize);
  }
}

bool isPrefix(const std::string& s, const std::string& prefix) {
  return s.substr(0, prefix.size()) == prefix;
}

std::string get_prefix(const std::string& s, size_t length) {
  std::string prefix;
  for (size_t i = 0; i < length; ++i) {
    prefix.push_back(s.size() > i ? s[i] : '#');
  }
  return prefix;
}

int SmartStorage::get_user(const login_t& login, const password_t& password) {
  Block& block = data["users"][get_prefix(login, 1)];
  
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
    try {
      auto result = std::stoi(value);
      return result;
    } catch (...) {
      log << "Unable stoi in get_user, value =[" << value << "]" << std::endl;
      throw;
    }
  }
  return -1;
}

int SmartStorage::get_user_count() {
  if (user_count != -1) {
    return user_count;
  }
  std::string path = config.get<std::string>("userdataPath");
  
  int blocksCount = fs::get_file_count(path);
  if (blocksCount == 0) {
    user_count = 0;
    return 0;
  }
  Block& block = data["userdata"][blocksCount - 1];
  std::stringstream ss(block[block.size() - 1]);
  std::string id;
  ss >> id;
  try {
    user_count = std::stoi(id);
  } catch (...) {
    log << "Unable to stoi in get_user_count, id=[" << id << "]" << std::endl;
    throw;
  }
  return user_count;
}

int SmartStorage::get_user_data_block(int id) {
  return (id - 1) / config.get<int>("userDataBlockSize");
}

int SmartStorage::get_user_data_block_pos(int id) {
  return (id - 1) % config.get<int>("userDataBlockSize");
}

int SmartStorage::add_user(const login_t& login, const password_t& password) {
  int user_status = get_user(login, password);
  if (user_status != -1) {
    return -1;
  }

  userid_t id = get_user_count() + 1;
  Block& block = data["users"][get_prefix(login, 1)];
  std::string s = login + " " + password + " " + std::to_string(id);
  block.add(s);
  block.save(config.get<std::string>("usersPath") + get_prefix(login, 1));

  int block_id = get_user_data_block(id);
  Block& data_block = data["userdata"][block_id];
  s = std::to_string(id) + " " + login;
  data_block.add(s);
  ++user_count;
  data_block.save(config.get<std::string>("userdataPath") + std::to_string(block_id));
  return id;
}

int SmartStorage::get_chats_count() {
  if (chat_count != -1) {
    return chat_count;
  }
  std::string path = config.get<std::string>("chatsPath");
  
  int block_count = fs::get_file_count(path);
  if (block_count == 0) {
    chat_count = 0;
    return 0;
  }

  chat_count = block_count;
  return chat_count;
}

bool SmartStorage::is_member(Block& block, userid_t member) {
  return block[0].find(" " + std::to_string(member) + " ") != std::string::npos;
}

bool SmartStorage::is_member(chatid_t chat, userid_t member) {
  Block& block = data["chats"][std::to_string(chat)];
  return is_member(block, member);
}

int SmartStorage::create_chat(userid_t creator) {
  chatid_t id = get_chats_count() + 1;
  Block& block = data["chats"][std::to_string(id)];
  block[0] = " " + std::to_string(creator) + " ";
  block[1] = "0";
  block.save();
  add_available_chat(creator, id);
  ++chat_count;
  return id;
}

void SmartStorage::add_available_chat(userid_t id, chatid_t chat) {
  Block& block = data["availableChats"][get_user_data_block(id)];
  block[get_user_data_block_pos(id)] += std::to_string(chat) + " ";
  block.block_size = std::max(block.size(), static_cast<size_t>(get_user_data_block_pos(id) + 1));
  block.save();
}

int SmartStorage::invite_to_chat(userid_t self_id, userid_t target, chatid_t chat) {
  if (target == self_id || target > user_count) {
    return -4;
  }

  if (chat > chat_count) {
    return -1;
  }

  Block& block = data["chats"][std::to_string(chat)];
  if (!is_member(block, self_id)) {
    return -2;
  }

  if (!is_member(block, target)) {
    add_available_chat(target, chat);
    block[0] += std::to_string(target) + " ";
    block.save();
  }

  return 0;
}

chatid_t SmartStorage::get_chat(userid_t self_id) {
  if (!current_chat.count(self_id)) {
    current_chat[self_id] = 0;
    chat_listeners[0].insert(self_id);
  }
  return current_chat[self_id];
}

int SmartStorage::get_user_chat(userid_t id, chatid_t chat) {
  if (chat < 0 || chat > get_chats_count()) {
    return -1;
  };

  if (chat != 0 && !is_member(chat, id)) {
    return -2;
  };

  if (current_chat.count(id)) {
    chat_listeners[current_chat[id]].erase(id);
  }

  current_chat[id] = chat;
  chat_listeners[chat].insert(id);
  return 0;
}

std::vector<chatid_t> SmartStorage::get_user_chats(userid_t id) {
  Block& block = data["availableChats"][get_user_data_block(id)];
  std::stringstream ss(block[get_user_data_block_pos(id)]);
  std::string chat_id;

  std::vector<chatid_t> chat_list;
  while (ss >> chat_id) {
    try {
      int chat_id_ = std::stoi(chat_id);
      chat_list.push_back(chat_id_);
    } catch (...) {
      log << "Failed stoi at get_user_chats, chatid=[" << chat_id << "]" << std::endl;
      throw;
    }
    
  }
  return chat_list;
}
  
std::vector<userid_t> SmartStorage::get_user_friends(userid_t id) {
  Block& block = data["friends"][get_user_data_block(id)];
  std::stringstream ss(block[get_user_data_block_pos(id)]);
  std::string friendId;

  std::vector<userid_t> friend_list;
  while (ss >> friendId) {
    try {
      int friendId_ = std::stoi(friendId);
      friend_list.push_back(friendId_);
    } catch (...) {
      log << "Failed stoi at get_user_friends, chatid=[" << friendId << "]" << std::endl;
      throw;
    }
    
  }
  return friend_list;
}

bool SmartStorage::is_friend(const std::string& s, userid_t target) {
  return s.find(" " + std::to_string(target) + " ") != std::string::npos;
}

int SmartStorage::add_friend(userid_t self_id, userid_t target) {
  if (target == self_id || target > user_count) {
    return -1;
  }

  Block& block = data["friends"][get_user_data_block(self_id)];
  std::string& s = block[get_user_data_block_pos(self_id)];

  if (is_friend(s, target)) {
    return -2;
  }
  if (s.empty()) {
    s.push_back(' ');
  }
  s += std::to_string(target) + " ";
  block.block_size = std::max(block.size(),
                              static_cast<size_t>(get_user_data_block_pos(self_id)) + 1);

  block.save();
  return 0;
}

std::string SmartStorage::get_user_nickname(userid_t id) {
  Block& block = data["userdata"][get_user_data_block(id)];
  std::string s = block[get_user_data_block_pos(id)];
  std::stringstream ss(s);
  std::string value;
  ss >> value;
  ss >> value;
  return value;
}

int SmartStorage::get_msg_count() {
  if (message_count != -1) {
    return message_count;
  }

  std::string path = config.get<std::string>("messagesPath");
  
  int blocksCount = fs::get_file_count(path);
  if (blocksCount == 0) {
    message_count = 0;
    return 0;
  }
  
  Block& block = data["messages"][blocksCount - 1];
  std::string& s = block[block.size() - 1];
  Object obj = encoder.decode(s);
  message_count = obj.id;
  return message_count;
}

int SmartStorage::get_msg_block(int id) {
  return (id - 1) / config.get<int>("messageBlockSize");
}

int SmartStorage::get_msg_block_pos(int id) {
  return (id - 1) % config.get<int>("messageBlockSize");
}

void SmartStorage::set_message(Object object, Encoder& encoder, chatid_t chatid) {
  if (!object.has_id()) {
    object.set_id(get_msg_count() + 1);
    ++message_count;
  }

  Block& block = data["messages"][get_msg_block(object.id)];
  std::string& s = block[get_msg_block_pos(object.id)];
  s = encoder.encode(object);
  block.save();

  Block& idblock = data["messagechatid"][get_msg_block(object.id)];
  std::string& sid = idblock[get_msg_block_pos(object.id)];
  sid = std::to_string(chatid);
  idblock.save();
}

int SmartStorage::add_message(Object object, Encoder& encoder, chatid_t chatid) {
  Block& block = data["chats"][std::to_string(chatid)];
  int prev = 0;
  try {
    prev = std::stoi(block[1]);
  } catch (...) {
    log << "Failed stoi in add_message, value=[" << block[1] << "]" << std::endl;
  }
  if (prev)
    object.set_prev(prev);

  int id = get_msg_count() + 1;
  object.set_id(id);
  ++message_count;
  
  if (prev != 0) {
    Object p = get_message(prev);
    p.set_next(object.id);
    set_message(p, encoder, chatid);
  }
  set_message(object, encoder, chatid);
  block[1] = std::to_string(object.id);
  block.save();
  return id;
}

Object SmartStorage::get_message(int id) {
  Block& block = data["messages"][get_msg_block(id)];
  std::string& s = block[get_msg_block_pos(id)];
  if (s.empty()) {
    Object object;
    object.set_return_code(-1);
    return object;
  }
  
  return encoder.decode(s);
}

Object SmartStorage::get_last_message(chatid_t chatid) {
  Block& block = data["chats"][std::to_string(chatid)];
  int id = 0;
  try {
    id = std::stoi(block[1]);
  } catch (...) {
    log << "Failed stoi at get_last_message, value=[" << block[1] << "]" << std::endl;
    throw;
  }
  
  if (id == 0) {
    Object obj;
    obj.set_return_code(-1);
    return obj;
  }
  return get_message(id);
}

chatid_t SmartStorage::get_message_chat_id(int id) {
  Block& block = data["messagechatid"][get_msg_block(id)];
  std::string& s = block[get_msg_block_pos(id)];
  if (s.empty()) {
    Object object;
    object.set_return_code(-1);
    return -1;
  }

  chatid_t chatid = 0;
  try {
    chatid = std::stoi(s);
  } catch (...) {
    log << "Failed stoi at get_message_chat_id, value=[" << s << "]" << std::endl;
    throw;
  }
  return chatid;
}