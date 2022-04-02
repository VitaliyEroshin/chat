#include "storage.hpp"

RAMStorage::Chat::Chat(): id(0) {};

RAMStorage::Chat::Chat(chatid_t id): id(id) {};

chatid_t RAMStorage::generateChatId() {
  return chats.size() + 1;
}

int RAMStorage::createChat(userid_t creator) {
  if (!users.count(creator)) {
    return -1;
  }
  chatid_t id = generateChatId();
  chats[id] = Chat(id);
  chats[id].members.insert(creator);
  availableChats[creator].push_back(id);
  return id;
}

int RAMStorage::inviteToChat(userid_t selfId, userid_t target, chatid_t chat) {
    if (!chats.count(chat)) {
      return -1;
    }

    if (!chats[chat].members.count(selfId)) {
      return -2;
    }

    if (!users.count(selfId)) {
      return -3;
    }

    if (!users.count(target)) {
      return -4;
    }

    chats[chat].members.insert(target);
    availableChats[target].push_back(chat);
    return 0;
  }