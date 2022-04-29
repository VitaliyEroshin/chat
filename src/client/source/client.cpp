#include "client.hpp"

Client::Client(Encoder& encoder, fs::Config& config)
    : status(Status::offline), 
      config(config), 
      socket(Socket(config.get<int>("port"))), 
      ui(UserInterface(*this)), 
      encoder(encoder) 
{}

bool Client::setAddress(std::string ip, int port) {
  socket.setAddress(port);
  if (ip == "localhost")
    ip = "127.0.0.1";
  
  return cstd::inet_pton(AF_INET, ip.c_str(), &(socket.address.sin_addr));
}

void Client::setupAddress() {
  std::string ip = ui.askForm({1, 1}, {1, 16}, "Enter ip address: ");
  int port;
  try {
    port = std::stoi(ui.askForm({2, 1}, {1, 4}, "Enter the port: "));
  } catch (...) {}

  while (!setAddress(ip, port)) {
    ui.print({1, 1}, {3, 30}, "");
    ui.print({4, 4}, "IP or port invalid. Please, try again.");
    ip = ui.askForm({1, 1}, {1, 16}, "Enter ip address: ");
    
    try {
      port = std::stoi(ui.askForm({2, 1}, {1, 4}, "Enter the port: "));
    } catch (...) {}
  }
  status = Status::connecting;
}

int Client::connectToHost() {
  return cstd::connect(socket.descriptor, (cstd::sockaddr*)&(socket.address), sizeof(socket.address));
}

std::pair<std::string, std::string> Client::askAuthData() {
  // H - for horizontal, V - for vertical
  const size_t 
    usernameOffsetH = 1,
    usernameOffsetV = 1,
    usernameBoxH = 12,
    usernameBoxV = 1;

  std::string username = ui.askForm(
    {usernameOffsetV, usernameOffsetH}, 
    {usernameBoxV, usernameBoxH},
     "Username: "
  );

  const size_t
    passwordOffsetH = 1,
    passwordOffsetV = 1,
    passwordBoxH = 12,
    passwordBoxV = 1;

  std::string password = ui.askForm(
    {usernameOffsetV + passwordOffsetV, passwordOffsetH}, 
    {passwordBoxV, passwordBoxH}, 
    "Password: "
  );

  return std::make_pair(username, password);
}

Object Client::makeAuthAttempt(const std::string& username, const std::string& password) {
  const char splitter = 1;
  Object object;
  object.type = Object::Type::loginAttempt;
  object.content += username;
  object.content.push_back(splitter);
  object.content += password;
  return object;
}

int Client::printAuthResult(int code) {
  const size_t
    resultOffsetH = 1,
    resultOffsetV = 4;

  auto printResult = [this] (const std::string& message) {
    ui.print(
      {resultOffsetV, resultOffsetV},
      message
    );
  };

  switch (code) {
    case 0:
      printResult("Logged in!");
      break;

    case 1:
      printResult("Created new user!");
      break;

    case 2:
      printResult("Wrong password");
      return -1;
  }

  return 0;
}

int Client::auth() {
  auto [username, password] = askAuthData();
  auto attempt = makeAuthAttempt(username, password);
  socket.send(encoder.encode(attempt));

  std::string query = socket.read();
  attempt = encoder.decode(query);

  return printAuthResult(attempt.code);
}

void Client::sendText(const std::string& text) {
  Object object;
  object.content = text;
  object.type = Object::Type::text;
  socket.send(encoder.encode(object));
}

void Client::sendCommand(const std::string& text) {
  Object object;
  object.content = text;
  object.type = Object::Type::command;
  socket.send(encoder.encode(object));
}


void Client::initializeGUI() {
  ui.clearWindow();
}

int ceil(int a, int b) {
  return (a + b - 1) / b;
}

void Client::refreshMessages() {
  size_t space = ui.getWindowHeight() - 3 - chatspace;
  size_t width = ui.getWindowWidth() - 2;
  ui.print({1, 1}, {space, width}, "");
  auto it = data.head;

  while (it != data.objects.end()) {
    std::stringstream ss((*it).content);
    std::string s;
    while (std::getline(ss, s)) {
      size_t height = ceil(s.size(), width - 2);
      if (space < height)
        return;

      ui.print({space - height + 1, 1}, {height, width - 2}, s);
      space -= height;
    }
    ++it;
  }
}

int Client::session() {
  if (connect() < 0)
    return -1;

  if (auth() < 0)
    return -1;

  initializeGUI();
  listen();

  return 0;
}

int Client::connect() {
  setupAddress();
  std::atomic<bool> connecting(true);
  std::thread background(&Client::showBackground, this, std::ref(connecting));

  if (connectToHost() < 0) {
    cstd::sleep(
      config.get<int>("connectionBackgroundDuration")
    );

    connecting.store(false);

    status = Client::Status::failed;
    ui.print({1, 1}, "Connection failed");
    cstd::sleep(
      config.get<int>("connectionFailedMessageDuration")
    );
  }

  connecting.store(false);
  background.join();
  if (status == Client::Status::failed) {
    return -1;
  }

  status = Client::Status::authentification;
  ui.print({1, 1}, {1, 20}, "Connected!");
  cstd::usleep(
    config.get<int>("connectionSucceedMessageDuration") * 1000
  );
  ui.clearWindow();
  return 0;
}

void Client::showBackground(std::atomic<bool>& connecting) {
  ui.clearWindow();
  int i = 0;
  while (connecting.load()) {
    ui.print({1, 1}, {1, 20}, "Loading" + std::string(i++ + 1, '.'));
    i %= 5;
    cstd::usleep(config.get<int>("loadingBackgroundSpeed") * 1000);
  }
  ui.clearWindow();
}

void Client::refreshOutput() {
  while (run.load()) {
    if (update.load()) {
      refreshMessages();
      update.store(false);
    }
    cstd::usleep(config.get<int>("refreshOutputDelay") * 1000);
  }
}

void Client::listen() {
  update.store(false);
  run.store(true);
  std::thread userInputThread(&Client::readUserInput, this);
  std::thread serverReadThread(&Client::readServer, this);
  std::thread refreshOutputThread(&Client::refreshOutput, this);
  
  userInputThread.join();
  serverReadThread.join();
  refreshOutputThread.join();
}

void Client::readServer() {
  while (run.load()) {
    std::string encoded = socket.read();

    if (encoded.empty()) {
      // Disconnected.
      run.store(false);
      return;
    }

    Object object = encoder.decode(encoded);
    if (object.type == Object::Type::text) {
      bool scroll = (data.head == data.objects.begin());

      if (data.objects.empty()) {
        data.insert(object);
        scroll = false;
        update.store(true);

      } else if (object.hasReturnCode() && object.code == 4) {
        object.setPrev(data.objects.front().id);
        object.setId(data.objects.front().id);
        data.objects.push_front(object);

      } else if (object.prev == data.objects.front().id) {
        data.objects.push_front(object);

      } else if (object.id == data.objects.back().prev) {
        data.objects.push_back(object);
        scroll = false;

      }
      
      if (scroll) {
        scrolldown();
        update.store(true);
      }
    }
  }
}

void Client::readUserInput() {
  chatspace = 1;
  while (run.load()) {
    drawChatPointer();
    std::string command = ui.input(
      {ui.getWindowHeight() - 1 - chatspace, 4},
      {chatspace, ui.getWindowWidth() - 8}, 
      true
    );

    ui.print(
      {ui.getWindowHeight() - 1 - chatspace, 4}, 
      {chatspace, ui.getWindowWidth() - 4}, 
      ""
    );

    chatspace = 1;

    if (command == "/quit") {
      ui.~UserInterface();
      run.store(false);
      socket.~Socket();
      return;
    }

    if (command == "/refresh") {
      initializeGUI();
      continue;
    }

    if (command[0] == '/') {
      sendCommand(command);
      continue;
    }
    if (command.empty()) {
      continue;
    }

    sendText(command);
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

ObjectTree::ObjectTree() {
  head = objects.begin();
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

void Client::allocateChatSpace() {

}

void Client::deallocateChatSpace() {

}

void Client::drawChatPointer() {
  ui.print(
    {ui.out.window.height - 2 - chatspace, 1}, 
    {1, ui.getWindowWidth() - 2}, 
    ""
  );

  ui.print(
    {ui.out.window.height - 2 - chatspace, 2}, 
    {chatspace + 2, 2},
    "  >   "
  );
}
