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

class Storage {
public:
  virtual int getUser(const login_t& login, const password_t& password) = 0;
  virtual int addUser(const login_t& login, const password_t& password) = 0;

  virtual int createChat(userid_t creator) = 0;
  virtual int inviteToChat(userid_t selfId, userid_t target, chatid_t chat) = 0;
  virtual chatid_t getChat(userid_t selfId) = 0;
  
  virtual const User& getUserReference(userid_t id) = 0;
  
  virtual int setUserChat(userid_t id, chatid_t chat) = 0;
  virtual const std::vector<userid_t>& getUserFriends(userid_t id) = 0;
  virtual const std::vector<chatid_t>& getUserChats(userid_t id) = 0;
  virtual int addFriend(userid_t selfId, userid_t target) = 0;
};

class RAMStorage: public Storage {
  struct Chat {
    explicit Chat(chatid_t id);
    Chat();

    chatid_t id;
    objectid_t lastMessage;
    std::set<userid_t> members;
  };

  std::map<chatid_t, Chat> chats;
  std::map<userid_t, User> users;
  std::map<std::string, userid_t> userids;

  std::map<userid_t, chatid_t> currentChat;
  std::map<userid_t, std::vector<userid_t>> friendsList;
  std::map<userid_t, std::vector<chatid_t>> availableChats;

  userid_t generateUserId();
  chatid_t generateChatId();

public:
  int getUser(const login_t& login, const password_t& password) override;
  int addUser(const login_t& login, const password_t& password) override;

  int createChat(userid_t creator) override;
  int inviteToChat(userid_t selfId, userid_t target, chatid_t chat) override;
  chatid_t getChat(userid_t selfId) override;

  const User& getUserReference(userid_t id) override;

  int setUserChat(userid_t id, chatid_t chat) override;
  const std::vector<userid_t>& getUserFriends(userid_t id) override;
  const std::vector<chatid_t>& getUserChats(userid_t id) override;
  int addFriend(userid_t selfId, userid_t target) override;
};

struct Block {
    static const size_t block_size = 256;
    std::string block;
    std::vector<std::string> data;

    void save(const std::string& path);
    
    std::string& operator[](size_t i);

    Block(const std::string& path, const std::string& block);

    Block(const std::string& block);
    Block() {};
    ~Block() = default;
};

class LRUCache {
    size_t maxCacheSize;
    std::string path;
    std::list<Block> cache;
    std::unordered_map<std::string, std::list<Block>::iterator> iterators;
    
public:
    Block& operator[](const std::string& i);
    Block& operator[](size_t i);

    LRUCache(size_t size, const std::string& path);
};