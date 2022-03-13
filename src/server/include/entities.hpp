#pragma once
#include <string>

typedef u_int16_t userid_t;
typedef std::string login_t;
typedef std::string password_t;

class User {
  struct Profile {
    std::string nickname;
  };

  userid_t id;
  login_t login;
  password_t password;

  Profile profile;
public:
  User();
  User(userid_t id, const login_t& login, const password_t& password);
  
  bool checkPassword(const password_t& password);
};

typedef u_int64_t objectid_t;

struct Object {
  objectid_t id;
  userid_t author;

  objectid_t terminal;
  int timestamp;
  objectid_t parentId;

  std::string message;

  void editMessage();
};