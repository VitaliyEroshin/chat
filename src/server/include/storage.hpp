#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

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
};

class RAMStorage: public Storage {
  struct Chat {
    explicit Chat(chatid_t id);
    Chat();

    chatid_t id;
    objectid_t lastMessage;
    std::unordered_set<userid_t> members;
  };

  std::map<chatid_t, Chat> chats;
  std::map<userid_t, User> users;
  std::map<std::string, userid_t> userids;

  userid_t generateUserId();
  chatid_t generateChatId();

public:
  int getUser(const login_t& login, const password_t& password) override;
  int addUser(const login_t& login, const password_t& password) override;

  int createChat(userid_t creator) override;
  int inviteToChat(userid_t selfId, userid_t target, chatid_t chat) override;
  chatid_t getChat(userid_t selfId) override { return 0; }

  const User& getUserReference(userid_t id) override {
    return users[id];
  }
};