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
  virtual int getUser(const login_t& login, const password_t& password) = 0;
  virtual int addUser(const login_t& login, const password_t& password) = 0;

  virtual int createChat(userid_t creator) = 0;
  virtual int inviteToChat(userid_t selfId, userid_t target, chatid_t chat) = 0;
  virtual chatid_t getChat(userid_t selfId) = 0;
  
  virtual std::string getUserNickname(userid_t id) = 0;

  virtual int setUserChat(userid_t id, chatid_t chat) = 0;
  virtual std::vector<userid_t> getUserFriends(userid_t id) = 0;
  virtual std::vector<chatid_t> getUserChats(userid_t id) = 0;
  virtual int addFriend(userid_t selfId, userid_t target) = 0;

  virtual void setMessage(Object object, Encoder& encoder, chatid_t chatid) = 0;
  virtual void addMessage(Object object, Encoder& encoder, chatid_t chatid) = 0;
  virtual Object getMessage(int id, Encoder& encoder) = 0;
  virtual Object getLastMessage(Encoder& encoder, chatid_t chatid) = 0;
  virtual chatid_t getMessageChatid(int id) = 0;
  virtual bool isMember(chatid_t chat, userid_t member) = 0;
};

// class RAMStorage: public Storage {
//   struct Chat {
//     explicit Chat(chatid_t id);
//     Chat();

//     chatid_t id;
//     objectid_t lastMessage;
//     std::set<userid_t> members;
//   };

//   std::map<chatid_t, Chat> chats;
//   std::map<userid_t, User> users;
//   std::map<std::string, userid_t> userids;

//   std::map<userid_t, chatid_t> currentChat;
//   std::map<userid_t, std::vector<userid_t>> friendsList;
//   std::map<userid_t, std::vector<chatid_t>> availableChats;

//   userid_t generateUserId();
//   chatid_t generateChatId();
//   const User& getUserReference(userid_t id);

// public:
//   int getUser(const login_t& login, const password_t& password) override;
//   int addUser(const login_t& login, const password_t& password) override;

//   int createChat(userid_t creator) override;
//   int inviteToChat(userid_t selfId, userid_t target, chatid_t chat) override;
//   chatid_t getChat(userid_t selfId) override;

//   std::string getUserNickname(userid_t id) override;

//   int setUserChat(userid_t id, chatid_t chat) override;
//   std::vector<userid_t> getUserFriends(userid_t id) override;
//   std::vector<chatid_t> getUserChats(userid_t id) override;
//   int addFriend(userid_t selfId, userid_t target) override;
// };

struct Block {
    std::string savePath;
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
    size_t maxCacheSize;
    std::string path;
    std::list<Block> cache;
    std::unordered_map<std::string, std::list<Block>::iterator> iterators;
    
public:
    Block& operator[](const std::string& i);
    Block& operator[](size_t i);

    LRUCache(size_t size, const std::string& path);
};

class SmartStorage : public Storage {
public:
  int getUser(const login_t& login, const password_t& password) override;
  int addUser(const login_t& login, const password_t& password) override;

  int createChat(userid_t creator) override;
  int inviteToChat(userid_t selfId, userid_t target, chatid_t chat) override;
  chatid_t getChat(userid_t selfId) override;
  int setUserChat(userid_t id, chatid_t chat) override;
  
  std::vector<userid_t> getUserFriends(userid_t id) override;
  std::vector<chatid_t> getUserChats(userid_t id) override;
  int addFriend(userid_t selfId, userid_t target) override;
  SmartStorage(const std::string& configPath, Logger& logger);

  std::string getUserNickname(userid_t id) override;

  void setMessage(Object object, Encoder& encoder, chatid_t chatid) override;
  void addMessage(Object object, Encoder& encoder, chatid_t chatid) override;
  Object getMessage(int id, Encoder& encoder) override;
  Object getLastMessage(Encoder& encoder, chatid_t chatid) override;
  chatid_t getMessageChatid(int id) override;
  bool isMember(chatid_t chat, userid_t member) override;

private:
  
  int getUserCount();
  int getUserDataBlock(size_t id);
  int getUserDataBlockPosition(size_t id);

  int getMessageCount();
  int getMessageBlock(int id);
  int getMessageBlockPosition(int id);

  std::string& getUserDataById(size_t id);

  int getChatsCount();
  bool isMember(Block& block, userid_t member);

  bool isFriend(const std::string&, userid_t target);

  void addAvailableChat(userid_t id, chatid_t chat);

  int userCount = -1;
  int chatCount = -1;
  int messageCount = -1;

  std::map<userid_t, chatid_t> currentChat;
  std::map<chatid_t, std::set<userid_t>> chatListeners;

  Logger& log;
  fs::Config config;
  LRUCache users;
  LRUCache userdata;
  LRUCache friends;
  LRUCache chats;
  LRUCache availableChats;
  LRUCache messages;
  LRUCache messagechatid;
};
