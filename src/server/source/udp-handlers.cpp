#include "udp-server.hpp"
#include <chrono>

void UdpServer::init_handlers() {
  add_handler("/addfriend", &UdpServer::addFriendHandler);
  add_handler("/myid", &UdpServer::getSelfIdHandler);
  add_handler("/chat", &UdpServer::getChatIdHandler);
  add_handler("/makechat", &UdpServer::makeChatHandler);
  add_handler("/invite", &UdpServer::inviteToChatHandler);
  add_handler("/switchchat", &UdpServer::switchChatHandler);
  add_handler("/friends", &UdpServer::getFriendsHandler);
  add_handler("/chats", &UdpServer::getChatsHandler);
  add_handler("/help", &UdpServer::getHelpHandler);
  add_handler("/about", &UdpServer::getAboutHandler);
  add_handler("/scrollup", &UdpServer::scrollUpHandler);
  add_handler("/scrolldown", &UdpServer::scrollDownHandler);
}

template<typename Handler>
void UdpServer::add_handler(const std::string& command, Handler handler) {
    handlers[command] = std::bind(handler, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void UdpServer::addFriendHandler(Object& callback, Connection& user, std::stringstream& ss) {
  userid_t target;
  ss >> target;

  switch (storage.add_friend(user.user, target)) {
    case 0:
      callback.content = "Success";
      break;

    case -1:
      callback.content = "No such user found";
      break;

    case -2:
      callback.content = "User is already in your friendlist";
      break;

    default:
      callback.content = "Internal server error";
  };
}

void UdpServer::getSelfIdHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = "Your ID is: " + std::to_string(user.user);
}

void UdpServer::getChatIdHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = "You are in chat with ID: " + std::to_string(storage.get_chat(user.user));
}

void UdpServer::makeChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t id = storage.create_chat(user.user);
  storage.get_user_chat(user.user, id);
  callback.content = "You have successfully created chat with ID: " + std::to_string(id);
}

void UdpServer::inviteToChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t currentChat = storage.get_chat(user.user);
  if (!currentChat) {
    callback.content = "You cannot invite users to the global chat";
  } else {
    userid_t target;
    ss >> target;
    
    int code = storage.invite_to_chat(user.user, target, currentChat);
    if (code == -4) {
      callback.content = "User id is invalid";
    } else if (code == -1) {
      callback.content = "You are in invalid chat";
    } else if (code == -2) {
      callback.content = "You have not permission to invite people in that chat.";
    } else {
      callback.content = "You have invited " 
        + storage.get_user_nickname(target)
        + " to chat " + std::to_string(currentChat);
    }
  }
}

void UdpServer::switchChatHandler(Object& callback, Connection& user, std::stringstream& ss) {
  chatid_t id;
  ss >> id;
  int code = storage.get_user_chat(user.user, id);
  if (code == -1) {
    callback.content = "No chat found.";
  } else if (code == -2) {
    callback.content = "You are not a member of the chat";
  } else {
    callback.content = "You have succesfully switched the chat";
    callback.set_return_code(6);
  }
}

void UdpServer::getFriendsHandler(Object& callback, Connection& user, std::stringstream& ss) {
  const std::vector<userid_t>& friends = storage.get_user_friends(user.user);
  if (friends.empty()) {
    callback.content = "You have no friends :(";
    return;
  }

  callback.content = "Your friends are: ";
  for (auto &usr : friends) {
    callback.content += storage.get_user_nickname(usr) + "(" + std::to_string(usr) + ")";
    if (usr != friends.back()) {
      callback.content += ",";
    } else {
      callback.content += ".";
    }
  }
}

void UdpServer::getChatsHandler(Object& callback, Connection& user, std::stringstream& ss) {
  const std::vector<chatid_t>& userchats = storage.get_user_chats(user.user);
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

void UdpServer::getHelpHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = fs::load_content("./content/help/header.txt");
  socket.send(encoder.encode(callback), user.address);
  callback.content = fs::load_content("./content/help/friends.txt");
  socket.send(encoder.encode(callback), user.address);
  callback.content = fs::load_content("./content/help/chats.txt");
  socket.send(encoder.encode(callback), user.address);
  callback.content = fs::load_content("./content/help/other.txt");
  socket.send(encoder.encode(callback), user.address);
  callback.content = fs::load_content("./content/help/footer.txt");
}

void UdpServer::getAboutHandler(Object& callback, Connection& user, std::stringstream& ss) {
  callback.content = fs::load_content("./content/about.txt");
}

void UdpServer::add_message_handler(Object& object, Connection& user, std::stringstream& ss) {
  chatid_t chat = storage.get_chat(user.user);
  
  // TODO
  // auto timestamp = getTimestamp();
  // object.set_timestamp(timestamp);

  object.content = "[" + storage.get_user_nickname(user.user) + "] " + object.content;
  
  if (chat != 0) {
    log << "Attaching object " << object.info();
    int id = storage.add_message(object, encoder, chat);
    log << "OK" << std::endl;
    object = storage.get_message(id, encoder);
  } else {
    object.set_return_code(4);
  }

  for (auto &[ip, other] : ip_to_connection) {
    if (storage.get_chat(other.user) != storage.get_chat(user.user)) {
      continue;
    }

    socket.send(encoder.encode(object), other.address);
  }
}

void UdpServer::scrollUpHandler(Object& object, Connection& user, std::stringstream& ss) {
  chatid_t chat = storage.get_chat(user.user);
  if (chat == 0) {
    return;
  }
  auto lastMessage = storage.get_last_message(encoder, chat);
  if (lastMessage.code == -1) {
    object.content = "This is the beginning of chat.";
    object.set_return_code(4);
    return;
  }

  if (!lastMessage.has_id()) {
    return;
  }

  if (!storage.is_member(chat, user.user)) {
    return;
  }

  log << "Called scrollup " << object.info() << std::endl;

  log << "In chat " << chat << std::endl; 

  Object obj = storage.get_message(object.id == 0 ? lastMessage.id : object.id, encoder);
  
  log << "Last message " << obj.info() << std::endl;

  const size_t
    kHistoricMessage = 5;

  obj.set_return_code(kHistoricMessage);
  obj.type = Object::Type::text;

  if (object.id == 0)
    socket.send(encoder.encode(obj), user.address);

  const size_t callbackSize = 2;
  for (size_t i = 0; i < callbackSize; ++i) {
    obj = storage.get_message(obj.prev, encoder);
    obj.set_return_code(kHistoricMessage);
    if (!obj.has_prev() || obj.prev == 0) {
      break;
    }
    log << "Sending... " << obj.info();
    socket.send(encoder.encode(obj), user.address);
    log << "OK" << std::endl;
  }
  object = obj;

}

void UdpServer::scrollDownHandler(Object& object, Connection& user, std::stringstream& ss) {
  if (!object.has_id()) {
    return;
  }
  chatid_t chat = storage.get_message_chat_id(object.id);
  if (!storage.is_member(chat, user.user)) {
    return;
  }
  
  Object obj = storage.get_message(object.id, encoder);
  socket.send(encoder.encode(obj), user.address);
  for (size_t i = 0; i < 3; ++i) {
    if (!obj.has_next() || obj.next == 0) {
      break;
    }
    obj = storage.get_message(obj.next, encoder);
    socket.send(encoder.encode(obj), user.address);
  }
  object = Object();
  object.type = Object::Type::returnCode;
}