#include "server.hpp"

void Server::initHandlers() {
    addHandler("/addfriend", &Server::addFriendHandler);
    addHandler("/myid", &Server::getSelfIdHandler);
    addHandler("/chat", &Server::getChatIdHandler);
    addHandler("/makechat", &Server::makeChatHandler);
    addHandler("/invite", &Server::inviteToChatHandler);
    addHandler("/switchchat", &Server::switchChatHandler);
    addHandler("/friends", &Server::getFriendsHandler);
    addHandler("/chats", &Server::getChatsHandler);
    addHandler("/help", &Server::getHelpHandler);
    addHandler("/about", &Server::getAboutHandler);
}

template<typename Handler>
void Server::addHandler(const std::string& command, Handler handler) {
    handlers[command] = std::bind(handler, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void Server::addFriendHandler(Object& callback, Connection& user, std::stringstream& ss) {
  userid_t target;
  ss >> target;
  int code = storage.addFriend(user.user, target);

  if (code == -1) {
    callback.message = "No such user found";
  }
}

void Server::getSelfIdHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.message = "Your ID is: " + std::to_string(user.user);
}

void Server::getChatIdHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.message = "You are in chat with ID: " + std::to_string(storage.getChat(user.user));
}

void Server::makeChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t id = storage.createChat(user.user);
  storage.setUserChat(user.user, id);
  callback.message = "You have successfully created chat with ID: " + std::to_string(id);
}

void Server::inviteToChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t currentChat = storage.getChat(user.user);
  if (!currentChat) {
    callback.message = "You cannot invite users to the global chat";
  } else {
    userid_t target;
    ss >> target;
    storage.inviteToChat(user.user, target, currentChat);
    callback.message = "You have invited " + storage.getUserReference(target).getNickname() 
      + " to chat " + std::to_string(currentChat);
  }
}

void Server::switchChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t id;
  ss >> id;
  int code = storage.setUserChat(user.user, id);
  if (code == -1) {
    callback.message = "No chat found.";
  } else if (code == -2) {
    callback.message = "You are not a member of the chat";
  } else {
    callback.message = "You have succesfully switched the chat";
  }
}

void Server::getFriendsHandler(Object& callback, Connection& user, std::stringstream& ss) {
  const std::vector<userid_t>& friends = storage.getUserFriends(user.user);
  if (friends.empty()) {
    callback.message = "You have no friends :(";
    return;
  }

  callback.message = "Your friends are: ";
  for (auto &usr : friends) {
    callback.message += storage.getUserReference(usr).getNickname();
    if (usr != friends.back()) {
      callback.message += ",";
    }
  }
}

void Server::getChatsHandler(Object& callback, Connection& user, std::stringstream& ss) {
  const std::vector<chatid_t>& userchats = storage.getUserChats(user.user);
  if (userchats.empty()) {
    callback.message = "You have no available chats :(";
    return;
  }

  callback.message += "Your available chats are: ";
  for (auto &cht : userchats) {
    callback.message += std::to_string(cht);
    if (cht != userchats.back()) {
      callback.message += ", ";
    }
  }
}

void Server::getHelpHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.message = fs::loadContent("./content/help.txt");
}

void Server::getAboutHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.message = fs::loadContent("./content/about.txt");
}