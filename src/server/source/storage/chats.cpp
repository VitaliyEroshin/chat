#include "storage.hpp"

Storage::Chat::Chat(): id(0) {};

Storage::Chat::Chat(chatid_t id): id(id) {};

chatid_t Storage::generateChatId() {
  return chats.size();
}

int Storage::createChat(userid_t creator) {
  if (!users.count(creator)) {
    return -1;
  }
  chatid_t id = generateChatId();
  chats[id] = Chat(id);
  chats[id].members.insert(creator);
  return id;
}

int Storage::inviteToChat(userid_t selfId, userid_t target, chatid_t chat) {
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
    return 0;
  }