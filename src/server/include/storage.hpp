#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

#include "entities.hpp"
#include "encoder.hpp"

class Storage {
  struct Chat {
    Chat(chatid_t id);
    Chat();

    chatid_t id;
    objectid_t lastMessage;
    std::unordered_set<userid_t> members;
  };

  struct Element {
    Object object;
    objectid_t next;
  };

public:
  std::map<chatid_t, Chat> chats;
  std::map<userid_t, User> users;
  std::map<std::string, userid_t> userids;

  int getUser(const login_t& login, const password_t& password);
  int addUser(const login_t& login, const password_t& password);

  userid_t generateUserId();
  chatid_t generateChatId();

  int createChat(userid_t creator);

  int inviteToChat(userid_t selfId, userid_t target, chatid_t chat);
};