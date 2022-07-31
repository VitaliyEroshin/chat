#pragma once

#include "server-base.hpp"

using handler_t = std::function<
        void(std::stringstream&, ConnectionBase&, const Object&,
             const std::function<void(const Object&)>&, Storage&)>;

struct Handlers {
  static handler_t add_friend_handler;
  static handler_t get_self_id_handler;
  static handler_t get_chat_id_handler;
  static handler_t make_chat_handler;
  static handler_t invite_to_chat_handler;
  static handler_t switch_chat_handler;
  static handler_t get_friends_handler;
  static handler_t get_chats_handler;
  static handler_t get_help_handler;
  static handler_t get_about_handler;
  static handler_t scroll_up_handler;
  static handler_t scroll_down_handler;
  static handler_t quit_handler;
};