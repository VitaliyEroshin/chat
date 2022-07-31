#include "constants.hpp"

std::map<std::string, std::any> ServerConfig::get_default_values() {
  // Net constants
  int port = 8888;

  // Storage constants
  std::string usersPath = "./data/users/";
  std::string userdataPath = "./data/userdata";
  int userDataBlockSize = 2;
  std::string chatsPath = "./data/chats/";

  std::string availableChatsPath = "./data/availablechats/";
  std::string friendsPath = "./data/friends/";
  std::string messagesPath = "./data/messages/";
  int messageBlockSize = 2;
  std::string messagechatidPath = "./data/messagechatid/";

  std::map<std::string, std::any> default_values;
  default_values.insert({"port", port});
  default_values.insert({"usersPath", usersPath});
  default_values.insert({"userdataPath", userdataPath});
  default_values.insert({"userDataBlockSize", userDataBlockSize});
  default_values.insert({"chatsPath", chatsPath});
  default_values.insert({"availableChatsPath", availableChatsPath});
  default_values.insert({"friendsPath", friendsPath});
  default_values.insert({"messagesPath", messagesPath});
  default_values.insert({"messageBlockSize", messageBlockSize});
  default_values.insert({"messagechatidPath", messagechatidPath});

  return default_values;
}