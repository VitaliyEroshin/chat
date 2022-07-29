#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <fstream>
#include <list>
#include "entities.hpp"
#include "encoder.hpp"
#include "filesystem.hpp"
#include "logger.hpp"

class Storage {
public:
  virtual int get_user(const login_t& login, const password_t& password) = 0;
  virtual int add_user(const login_t& login, const password_t& password) = 0;

  virtual int create_chat(userid_t creator) = 0;
  virtual int invite_to_chat(userid_t selfId, userid_t target, chatid_t chat) = 0;
  virtual chatid_t get_chat(userid_t selfId) = 0;
  
  virtual std::string get_user_nickname(userid_t id) = 0;

  virtual int get_user_chat(userid_t id, chatid_t chat) = 0;
  virtual std::vector<userid_t> get_user_friends(userid_t id) = 0;
  virtual std::vector<chatid_t> get_user_chats(userid_t id) = 0;
  virtual int add_friend(userid_t selfId, userid_t target) = 0;

  virtual void set_message(Object object, Encoder& encoder, chatid_t chatid) = 0;
  virtual int add_message(Object object, Encoder& encoder, chatid_t chatid) = 0;
  virtual Object get_message(int id, Encoder& encoder) = 0;
  virtual Object get_last_message(Encoder& encoder, chatid_t chatid) = 0;
  virtual chatid_t get_message_chat_id(int id) = 0;
  virtual bool is_member(chatid_t chat, userid_t member) = 0;
};

struct Block {
    std::string save_path;
    size_t block_size;
    std::string block;
    std::vector<std::string> data;

    void save(const std::string& path);
    void save();

    size_t size();

    std::string& operator[](size_t i);
    
    void add(const std::string& s);

    Block(const std::string& path, const std::string& block);

    Block(const std::string& block);
    Block() {};
    ~Block() = default;
};

class LRUCache {
    size_t max_cache_size{};
    std::string path;
    std::list<Block> cache;
    std::unordered_map<std::string, std::list<Block>::iterator> iterators;
    
public:
    Block& operator[](const std::string& i);
    Block& operator[](size_t i);

    LRUCache(size_t size, const std::string& path);
    LRUCache() = default;
    LRUCache(LRUCache&& other) noexcept ;
};

class SmartStorage : public Storage {
public:
  int get_user(const login_t& login, const password_t& password) override;
  int add_user(const login_t& login, const password_t& password) override;

  int create_chat(userid_t creator) override;
  int invite_to_chat(userid_t self_id, userid_t target, chatid_t chat) override;
  chatid_t get_chat(userid_t self_id) override;
  int get_user_chat(userid_t id, chatid_t chat) override;
  
  std::vector<userid_t> get_user_friends(userid_t id) override;
  std::vector<chatid_t> get_user_chats(userid_t id) override;
  int add_friend(userid_t self_id, userid_t target) override;

  void add_module(const std::string& alias, size_t lru_size);
  SmartStorage(const std::string& cfg_path, Logger& logger, Encoder& encoder);

  std::string get_user_nickname(userid_t id) override;

  void set_message(Object object, Encoder& encoder, chatid_t chatid) override;
  int add_message(Object object, Encoder& encoder, chatid_t chatid) override;
  Object get_message(int id, Encoder& encoder) override;
  Object get_last_message(Encoder& encoder, chatid_t chatid) override;
  chatid_t get_message_chat_id(int id) override;
  bool is_member(chatid_t chat, userid_t member) override;

private:
  
  int get_user_count();
  int get_user_data_block(int id);
  int get_user_data_block_pos(int id);

  int get_msg_count();
  int get_msg_block(int id);
  int get_msg_block_pos(int id);

  std::string& get_user_data_by_id(size_t id);

  int get_chats_count();
  bool is_member(Block& block, userid_t member);

  static bool is_friend(const std::string &s, userid_t target);

  void add_available_chat(userid_t id, chatid_t chat);

  int user_count = -1;
  int chat_count = -1;
  int message_count = -1;

  std::map<userid_t, chatid_t> current_chat;
  std::map<chatid_t, std::set<userid_t>> chat_listeners;

  Logger& log;
  fs::Config config;
  Encoder& encoder;
  std::unordered_map<std::string, LRUCache> data;
};
