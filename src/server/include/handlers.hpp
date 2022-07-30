#pragma once

#include "server-base.hpp"

#include <chrono>

inline uint64_t get_timestamp() {
  using namespace std::chrono;
  auto timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  return timestamp;
}

inline Object make_callback_obj() {
  Object callback;
  callback.type = Object::Type::text;

  const int kCommandCallback = 4;
  callback.set_return_code(kCommandCallback);
  return callback;
}

auto add_friend_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
                const std::function<void(const Object&)>& send, Storage& storage)
{
  userid_t target;
  request >> target;

  Object callback = make_callback_obj();

  switch (storage.add_friend(user_data.user, target)) {
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

  send(callback);
};

auto get_self_id_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  callback.content = "Your ID is: " + std::to_string(user_data.user);
  send(callback);
};

auto get_chat_id_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  callback.content = "You are in chat with ID: " + std::to_string(storage.get_chat(user_data.user));
  send(callback);
};

auto make_chat_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  chatid_t id = storage.create_chat(user_data.user);
  storage.get_user_chat(user_data.user, id);
  callback.content = "You have successfully created chat with ID: " + std::to_string(id);
  send(callback);
};

auto invite_to_chat_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  chatid_t currentChat = storage.get_chat(user_data.user);
  if (!currentChat) {
    callback.content = "You cannot invite users to the global chat";
    send(callback);
    return;
  }

  userid_t target;
  request >> target;

  int code = storage.invite_to_chat(user_data.user, target, currentChat);
  switch (storage.invite_to_chat(user_data.user, target, currentChat)) {
    case -4:
      callback.content = "User id is invalid.";
      break;

    case -1:
      callback.content = "You are in invalid chat.";
      break;

    case -2:
      callback.content = "You have not permission to invite people in that chat.";
      break;

    default:
      callback.content = "You have invited "
                         + storage.get_user_nickname(target)
                         + " to chat " + std::to_string(currentChat);
  }

  send(callback);
};

auto switch_chat_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  chatid_t id;
  request >> id;
  auto callback = make_callback_obj();

  switch (storage.get_user_chat(user_data.user, id)) {
    case -1:
      callback.content = "No chat found.";
      break;

    case -2:
      callback.content = "You are not a member of the chat";
      break;

    default:
      callback.content = "You have succesfully switched the chat";
      callback.set_return_code(6);

  }

  send(callback);
};

auto get_friends_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  const std::vector<userid_t>& friends = storage.get_user_friends(user_data.user);
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

  send(callback);
};

auto get_chats_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  const std::vector<chatid_t>& userchats = storage.get_user_chats(user_data.user);
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
  send(callback);
};

auto get_help_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  for (auto module : std::vector<std::string>{"header", "friends", "chats", "other", "footer"}) {
    callback.content = fs::load_content("./content/help/" + module +  ".txt");
    send(callback);
  }
};

auto get_about_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  auto callback = make_callback_obj();
  callback.content = fs::load_content(("./content/about.txt"));
  send(callback);
};

auto scroll_up_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  chatid_t chat = storage.get_chat(user_data.user);
  if (chat == 0) {
    return;
  }

  auto last_message = storage.get_last_message(chat);
  if (last_message.code == -1) {
    auto callback = make_callback_obj();
    callback.content = "This is the beginning of chat.";
    callback.set_return_code(4);
    send(callback);
    return;
  }

  if (!last_message.has_id() || !storage.is_member(chat, user_data.user)) {
    return;
  }

  Object callback = storage.get_message(meta.id == 0 ? last_message.id : meta.id);

  const size_t kHistoricMessage = 5;
  const size_t callbackSize = 2;

  callback.set_return_code(kHistoricMessage);
  callback.type = Object::Type::text;

  if (meta.id == 0)
    send(callback);

  for (size_t i = 0; i < callbackSize; ++i) {
    callback = storage.get_message(callback.prev);
    callback.set_return_code(kHistoricMessage);
    if (!callback.has_prev() || callback.prev == 0) {
      break;
    }
    send(callback);
  }
  send(callback);
};

auto scroll_down_handler =
        [](std::stringstream& request, ConnectionBase& user_data, const Object& meta,
           const std::function<void(const Object&)>& send, Storage& storage)
{
  chatid_t chat = storage.get_message_chat_id(meta.id);

  if (!meta.has_id() || !storage.is_member(chat, user_data.user)) {
    return;
  }

  Object callback = storage.get_message(meta.id);
  send(callback);

  const size_t callbackSize = 2;

  for (size_t i = 0; i < callbackSize; ++i) {
    if (!callback.has_next() || callback.next == 0) {
      break;
    }
    callback = storage.get_message(callback.next);
    send(callback);
  }
};