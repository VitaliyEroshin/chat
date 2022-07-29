#include "client.hpp"

Client::Client(Encoder& encoder, fs::Config& config, Logger& logger)
    : status(Status::offline), 
      config(config), 
      socket(Socket(config.get<int>("port"))), 
      ui(UserInterface(*this)), 
      encoder(encoder),
      log(logger)
{}

bool Client::set_address(std::string ip, int port) {
  socket.set_port(port);
  if (ip == "localhost")
    ip = "127.0.0.1";
  
  return socket.setup_address(ip);
}

std::pair<std::string, std::string> Client::ask_address() {
  static const size_t
    ip_offset_v = 1,
    ip_offset_h = 1,
    ip_box_v = 1,
    ip_box_h = 16;
  
  std::string ip = ui.ask_form(
          {ip_offset_v, ip_offset_h},
          {ip_box_v, ip_box_h},
          "Enter ip address: "
  );

  static const size_t
    port_offset_v = 1,
    port_offset_h = 1,
    port_box_v = 1,
    port_box_h = 5;

  std::string port = ui.ask_form(
          {ip_offset_v + port_offset_v, port_offset_h},
          {port_box_v, port_box_h},
          "Enter the port: "
  );

  return std::make_pair(ip, port);
}

void Client::show_address_hint() {
  static const size_t
    verdict_offset_v = 4,
    verdict_offset_h = 4;

  ui.print(
    {verdict_offset_v, verdict_offset_h},
    "Oops! You have entered wrong address."
  );

  static const size_t
    hint_offset_v = 6,
    hint_offset_h = 2;

    ui.print_lines(
            {hint_offset_v, hint_offset_h},
            {
                    "+--------+------------------------------------+",
                    "|  HINT  |                                    |",
                    "+--------+                                    |",
                    "|                                             |",
                    "|      IP address should be localhost or      |",
                    "|  follow the pattern (X.X.X.X) where X       |",
                    "|  is a number between 0 and 255.             |",
                    "|                                             |",
                    "|      A port number is a 16-bit unsigned     |",
                    "|  integer, thus ranging from 0 to 65535      |",
                    "|                                             |",
                    "+---------------------------------------------+"
            }
    );
}

void Client::setup_address() {
  std::string ip;
  int port = 0;
  bool hint = false;
  while (!set_address(ip, port)) {
    ui.clear_cli_window();
    if (hint)
        show_address_hint();

    hint = true;
    
    auto [ip_str, port_str] = ask_address();
    try {
      port = std::stoi(port_str);
      ip = ip_str;
    } catch (...) {
      log << "User entered not a number. Asking him again..." << std::endl;
      continue;
    }
  }

  status = Status::connecting;
}

std::pair<std::string, std::string> Client::ask_auth_data() {
  static const size_t
    uname_offset_v = 1,
    uname_offset_h = 1,
    uname_box_v = 1,
    uname_box_h = 12;

  std::string username = ui.ask_form(
          {uname_offset_v, uname_offset_h},
          {uname_box_v, uname_box_h},
          "Username: "
  );

  static const size_t
    pass_offset_v = 1,
    pass_offset_h = 1,
    pass_box_v = 1,
    pass_box_h = 12;

  std::string password = ui.ask_form(
          {uname_offset_v + pass_offset_v, pass_offset_h},
          {pass_box_v, pass_box_h},
          "Password: "
  );

  return std::make_pair(username, password);
}

Object Client::make_auth_attempt(const std::string& username, const std::string& password) {
  static const char splitter = 1;
  Object object;
  object.type = Object::Type::loginAttempt;
  object.content += username;
  object.content.push_back(splitter);
  object.content += password;
  return object;
}

int Client::print_auth_results(int code) {
  static const size_t
    res_offset_v = 4,
    res_offset_h = 1;

  auto print_result = [this] (const std::string& message) {
    ui.print(
      {res_offset_v, res_offset_h},
      message
    );
  };

  switch (code) {
    case 0:
      print_result("Logged in!");
      break;

    case 1:
      print_result("Created new user!");
      break;

    case 2:
      print_result("Wrong password");
      return -1;

    default:
      print_result("Unknown error");
      return -1;
  }

  return 0;
}

void Client::send_text(const std::string& text) {
  Object object;
  object.content = text;
  object.type = Object::Type::text;
  socket.send(encoder.encode(object));
}

void Client::send_command(const std::string& text) {
  Object object;
  object.content = text;
  object.type = Object::Type::command;
  socket.send(encoder.encode(object));
}

void Client::init_gui() {
  ui.clear_cli_window();
}

int ceil(int a, int b) {
  assert(a != 0 && b != 0);
  return (a + b - 1) / b;
}

bool Client::print_message(size_t& space, size_t width, const std::string& message) {
  static const size_t
    msg_box_top_pad = 1,
    msg_box_left_pad = 1;

  std::stringstream stream(message);
  std::string buffer;

  std::vector<std::string> lines;
  
  while (std::getline(stream, buffer)) {
    lines.push_back(buffer);
  }

  std::reverse(lines.begin(), lines.end());
  
  for (auto &line : lines) {
    size_t length = UserInterface::get_text_width(line);
    size_t height = ceil(static_cast<int>(length), static_cast<int>(width));

    if (space < height) return false;

    ui.print({space - height + msg_box_left_pad, msg_box_top_pad},
             {height, width}, line);

    space -= height;
  }
  return true;
}

void Client::refersh_messages() {
  static std::mutex mtx;
  std::lock_guard<std::mutex> lock(mtx);
  static const size_t
    msg_box_top_pad = 1,
    msg_box_bottom_pad = 1,
    chat_box_bottom_pad = 1,
    msg_box_left_pad = 1,
    msg_box_right_pad = 1;

  size_t space =
          ui.get_cli_window_height()
          - chat_box_bottom_pad
          - chatspace
          - msg_box_bottom_pad
          - msg_box_top_pad;

  size_t width =
          ui.get_cli_window_width()
          - msg_box_left_pad
          - msg_box_right_pad;

    ui.clear_space(
            {msg_box_top_pad, msg_box_left_pad},
            {space, width}
    );

  auto message = data.head;

  while (data.is_message(message)) {
    if (!print_message(space, width, (*message).content)) {
      return;
    }

    auto prev = message;
    ++message;
    if (!data.is_message(message) && (*prev).has_prev()) {
        get_previous_messages();
      return;
    }
  }
}

int Client::session() {
  if (connect() < 0)
    return -1;

  if (auth() < 0)
    return -1;

  init_gui();
  listen();

  return 0;
}

#include <unistd.h> // usleep

void Client::delay(const std::string& label) {
  static const size_t usec_in_msec = 1000;

  usleep(config.get<int>(label) * usec_in_msec);
}

void Client::show_background(std::atomic<bool>& connecting) {
  static const size_t
    loading_offset_v = 1,
    loading_offset_h = 1,
    loading_dots = 5;

  ui.clear_cli_window();
  int i = 0;

  while (connecting.load()) {
    ui.print(
      {loading_offset_v, loading_offset_h},
      "Loading" + std::string(i++ + 1, '.')
    );

    i %= loading_dots;
    delay("loadingBackgroundSpeed");
  }

  ui.clear_cli_window();
}

void Client::show_connection_verdict(const std::string& verdict) {
  static const size_t
    verdict_offset_v = 1,
    verdict_offset_h = 1;

  ui.print(
    {verdict_offset_v, verdict_offset_h},
    verdict
  );
}

void Client::refresh_output() {
  while (run.load()) {
    if (update.load()) {
        refersh_messages();
      update.store(false);
    }
    delay("refreshOutputDelay");
  }
}

void Client::listen() {
  update.store(false);
  run.store(true);
  std::thread usr_input_th(&Client::read_user_input, this);
  std::thread server_response_th(&Client::read_server, this);
  std::thread output_refresh_th(&Client::refresh_output, this);
  
  usr_input_th.join();
  server_response_th.join();
  output_refresh_th.join();
}


void Client::parse_text_object(Object object) {
  static const int
    kCommandCallback = 4,
    kHistoricMessage = 5,
    kChatSwitched = 6;

  bool scroll = (data.head == data.objects.begin());

  if (object.has_return_code(kChatSwitched)) {
    data.clear();
      send_command("/scrollup");

  } else if (data.objects.empty()) {
    data.insert(object);
    scroll = false;
    update.store(true);

  } else if (object.has_return_code(kCommandCallback)) {
    object.set_prev(data.frontId());
    object.set_id(data.frontId());
      data.push_front(object);

  } else if (object.id == data.objects.back().prev) {
      data.push_back(object);
    scroll = false;

  } else if (object.prev == data.objects.front().id) {
      data.push_front(object);

  } else if (data.objects.back().has_return_code(kCommandCallback) &&
             object.has_return_code(kHistoricMessage)) {
    data.objects.push_back(object);
      data.propagate_id_from_back();

  } else if (data.objects.front().has_return_code(kCommandCallback)) {
    data.objects.push_front(object);
  }

  update.store(true);
  
  if (scroll) {
    scrolldown();
  }
}

void Client::quit() {
  run.store(false);
  socket.~Socket();
}

void Client::parse_input_command(const std::string& command) {
  if (command == "/quit")
    quit();

  else if (command == "/refresh")
      init_gui();

  else if (command == "/scrollback")
      get_previous_messages();

  else if (command[0] == '/')
      send_command(command);
   
  else if (!command.empty())
      send_text(command);

}

void Client::read_user_input() {
  static const size_t
    chat_bottom_pad = 1,
    chat_offset_v = ui.get_cli_window_height()
                    - chat_bottom_pad
                    - chatspace,
    chat_offset_h = 4;

  chatspace = 1;
  while (run.load()) {
      draw_chat_ptr();
    std::string command = ui.input(
      {chat_offset_v, chat_offset_h},
      {chatspace, ui.get_cli_window_width() - 2 * chat_offset_h},
      true
    );

      ui.clear_space(
              {chat_offset_v, chat_offset_h},
              {chatspace, ui.get_cli_window_width() - chat_offset_h}
      );

    chatspace = 1;

      parse_input_command(command);
  }
}

void ObjectTree::insert(const Object& obj) {
  if (head == objects.end()) {
    objects.push_back(obj);
    head = objects.begin();
  } else {
    auto it = head;
    objects.insert(it, obj);
    --head;
  }
}

void ObjectTree::clear() {
  objects.clear();
  head = objects.begin();
}

ObjectTree::ObjectTree() {
  head = objects.begin();
}

bool ObjectTree::is_message(std::list<Object>::iterator message) const {
  return message != objects.end();
}

int ObjectTree::back_id() const {
  if (!objects.back().has_id())
    return -1;

  return objects.back().id;
}

int ObjectTree::frontId() const {
  if (!objects.front().has_id())
    return -1;

  return objects.front().id;
}

void ObjectTree::push_front(const Object& object) {
  objects.push_front(object);
}

void ObjectTree::push_back(const Object& object) {
  objects.push_back(object);
}

void ObjectTree::propagate_id_from_back() {
  static const size_t
    kCommandCallback = 4;

  auto it = --objects.end();
  while ((*it).has_return_code(kCommandCallback)) {
    (*it).set_prev(back_id());
    (*it).set_id(back_id());

    if (it == objects.begin()) break;
  }
}

void Client::get_previous_messages() {
  Object object;
  object.content = "/scrollup";
  object.set_id(data.back_id());
  object.type = Object::Type::command;
  socket.send(encoder.encode(object));
}

void Client::scrollup() {
  auto it = data.head;
  if (++it != data.objects.end()) {
    data.head = it;
    update.store(true);
  }
}

void Client::scrolldown() {
  if (data.head != data.objects.begin()) {
    data.head--;
    update.store(true);
  } else if (data.head != data.objects.end()) {
    // TODO
  }
}

void Client::alloc_chat_space() {
  // TODO
}

void Client::dealloc_chat_space() {
  // TODO
}

void Client::draw_chat_ptr() {
  ui.print(
    {ui.out.window.height - 2 - chatspace, 1}, 
    {1, ui.get_cli_window_width() - 2},
    ""
  );

  ui.print(
    {ui.out.window.height - 2 - chatspace, 2}, 
    {chatspace + 2, 2},
    "  >"
  );
}
