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
    addHandler("/scrollup", &Server::scrollUpHandler);
    addHandler("/scrolldown", &Server::scrollDownHandler);
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
    callback.content = "No such user found";
  }
}

void Server::getSelfIdHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = "Your ID is: " + std::to_string(user.user);
}

void Server::getChatIdHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = "You are in chat with ID: " + std::to_string(storage.getChat(user.user));
}

void Server::makeChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t id = storage.createChat(user.user);
  storage.setUserChat(user.user, id);
  callback.content = "You have successfully created chat with ID: " + std::to_string(id);
}

void Server::inviteToChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t currentChat = storage.getChat(user.user);
  if (!currentChat) {
    callback.content = "You cannot invite users to the global chat";
  } else {
    userid_t target;
    ss >> target;
    
    int code = storage.inviteToChat(user.user, target, currentChat);
    if (code == -4) {
      callback.content = "User id is invalid";
    } else if (code == -1) {
      callback.content = "You are in invalid chat";
    } else if (code == -2) {
      callback.content = "You have not permission to invite people in that chat.";
    } else {
      callback.content = "You have invited " 
        + storage.getUserNickname(target)
        + " to chat " + std::to_string(currentChat);
    }
  }
}

void Server::switchChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t id;
  ss >> id;
  int code = storage.setUserChat(user.user, id);
  if (code == -1) {
    callback.content = "No chat found.";
  } else if (code == -2) {
    callback.content = "You are not a member of the chat";
  } else {
    callback.content = "You have succesfully switched the chat";
  }
}

void Server::getFriendsHandler(Object& callback, Connection& user, std::stringstream& ss) {
  const std::vector<userid_t>& friends = storage.getUserFriends(user.user);
  if (friends.empty()) {
    callback.content = "You have no friends :(";
    return;
  }

  callback.content = "Your friends are: ";
  for (auto &usr : friends) {
    callback.content += storage.getUserNickname(usr) + "(" + std::to_string(usr) + ")";
    if (usr != friends.back()) {
      callback.content += ",";
    } else {
      callback.content += ".";
    }
  }
}

void Server::getChatsHandler(Object& callback, Connection& user, std::stringstream& ss) {
  const std::vector<chatid_t>& userchats = storage.getUserChats(user.user);
  if (userchats.empty()) {
    callback.content = "You have no available chats :(";
    return;
  }

  callback.content += "Your available chats are: ";
  for (auto &cht : userchats) {
    callback.content += std::to_string(cht);
    if (cht != userchats.back()) {
      callback.content += ", ";
    }
  }
}

void Server::getHelpHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = fs::loadContent("./content/help.txt");
}

void Server::getAboutHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = fs::loadContent("./content/about.txt");
}

void Server::addMessageHandler(Object& object, Connection& user, std::stringstream& ss) {
  chatid_t chat = storage.getChat(user.user);
  log << "Received message from " << user.user << " with content \"" << object.content << "\"\n";
  object.setAuthor(user.user);
  object.content = "[" + storage.getUserNickname(user.user) + "] " + object.content;
  if (chat != 0) {
    storage.addMessage(object, encoder, chat);
  } else {
    object.setReturnCode(4);
  }

  for (auto &other : connections) {
    if (storage.getChat(other.user) != storage.getChat(user.user)) {
      continue;
    }
    log << "Trying to send this to " << other.user << "\n";
    other.socket->send(encoder.encode(object));
  }
}

void Server::scrollUpHandler(Object& object, Connection& user, std::stringstream& ss) {
  log << "User " << user.user << " asked for scrollup.\n";
  chatid_t chat = storage.getChat(user.user);
  log << "Currently, user is in " << chat << ". Getting last message.\n";
  object = storage.getLastMessage(encoder, chat);
  if (!object.hasId()) {
    log << "Catched object does not have id. Ignoring.\n";
    return;
  }
  // chatid_t chat = storage.getMessageChatid(object.id);
  if (!storage.isMember(chat, user.user)) {
    log << "User is not a member of " << chat << ". Ignoring.\n";
    return;
  }
  log << "Sending messages: \n";
  Object obj = storage.getMessage(object.id, encoder);
  obj.type = Object::Type::text;
  user.socket->send(encoder.encode(obj));
  log << "Sent " << obj.content << "\n";
  for (size_t i = 0; i < 3; ++i) {
    if (!obj.hasPrev() || obj.prev == 0) {
      break;
    }
    obj = storage.getMessage(obj.prev, encoder);
    user.socket->send(encoder.encode(obj));
    log << "Sent " << obj.content << "\n";
  }
}

void Server::scrollDownHandler(Object& object, Connection& user, std::stringstream& ss) {
  if (!object.hasId()) {
    return;
  }
  chatid_t chat = storage.getMessageChatid(object.id);
  if (!storage.isMember(chat, user.user)) {
    return;
  }
  
  Object obj = storage.getMessage(object.id, encoder);
  user.socket->send(encoder.encode(obj));
  for (size_t i = 0; i < 3; ++i) {
    if (!obj.hasNext() || obj.next == 0) {
      break;
    }
    obj = storage.getMessage(obj.next, encoder);
    user.socket->send(encoder.encode(obj));
  }
  object = Object();
  object.type = Object::Type::returnCode;
}