#include "storage.hpp"
#include "entities.hpp"

User::User(userid_t id, const login_t& login, const password_t& password)
  : id(id), login(login), password(password), profile({login}) {};

User::User()
  : id(0), login(""), password("") {};

int RAMStorage::getUser(const login_t& login, const password_t& password) {
  if (!userids.count(login)) {
    return -1;
  }

  userid_t id = userids[login];
  if (!users[id].checkPassword(password)) {
    return -2;
  }

  return id;
}

int RAMStorage::addUser(const login_t& login, const password_t& password) {
  if (userids.count(login)) {
    return -1;
  }

  userid_t id = generateUserId();
  userids[login] = id;
  users[id] = User(id, login, password);
  return id;
}

userid_t RAMStorage::generateUserId() {
  return users.size() + 1;
}

const User& RAMStorage::getUserReference(userid_t id)  {
  return users[id];
}

chatid_t RAMStorage::getChat(userid_t selfId) {
  if (!currentChat.count(selfId)) {
    return 0;
  }
  return currentChat[selfId];
}

int RAMStorage::setUserChat(userid_t id, chatid_t chat) {
  if (chat == 0) {
    currentChat[id] = 0;
    return 0;
  }
  if (!chats.count(chat)) {
    return -1;
  }
  if (!chats[chat].members.count(id)) {
    return -2;
  }

  currentChat[id] = chat;
  return 0;
}

const std::vector<userid_t>& RAMStorage::getUserFriends(userid_t id) {
  return friendsList[id];
}

const std::vector<chatid_t>& RAMStorage::getUserChats(userid_t id) {
  return availableChats[id];
}

int RAMStorage::addFriend(userid_t selfId, userid_t target) {
  if (!users.count(target)) {
    return -1;
  }
  friendsList[selfId].push_back(target);
  return 0;
}