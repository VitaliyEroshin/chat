#pragma once
#include "types.hpp"
#include <string>

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
  
  bool checkPassword(const password_t& pass) {
    return pass == password;
  }
};