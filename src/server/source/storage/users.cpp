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
  return users.size();
}