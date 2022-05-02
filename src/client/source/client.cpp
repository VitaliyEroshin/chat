#include "client.hpp"

Client::Client(Encoder& encoder, fs::Config& config, Logger& logger)
    : status(Status::offline), 
      config(config), 
      socket(Socket(config.get<int>("port"))), 
      ui(UserInterface(*this)), 
      encoder(encoder),
      log(logger)
{}

bool Client::setAddress(std::string ip, int port) {
  socket.setAddress(port);
  if (ip == "localhost")
    ip = "127.0.0.1";
  
  return cstd::inet_pton(AF_INET, ip.c_str(), &(socket.address.sin_addr));
}

std::pair<std::string, std::string> Client::askAddress() {
  const size_t
    ipOffsetH = 1,
    ipOffsetV = 1,
    ipBoxH = 16,
    ipBoxV = 1;
  
  std::string ip = ui.askForm(
    {ipOffsetV, ipOffsetH}, 
    {ipBoxV, ipBoxH}, 
    "Enter ip address: "
  );

  const size_t
    portOffsetH = 1,
    portOffsetV = 1,
    portBoxH = 5,
    portBoxV = 1;

  std::string port = ui.askForm(
    {ipOffsetV + portOffsetV, portOffsetH}, 
    {portBoxV, portBoxH}, 
    "Enter the port: "
  );

  return std::make_pair(ip, port);
}

void Client::showAddressHint() {
  const size_t 
    verdictOffsetV = 4,
    verdictOffsetH = 4;

  ui.print(
    {verdictOffsetV, verdictOffsetH},
    "Oops! You have entered wrong address."
  );

  const size_t
    hintOffsetV = 6,
    hintOffsetH = 2;

  const std::string
    boldOn = "\e[0m",
    boldOff = "\e[1m";

  ui.printLines(
    {hintOffsetV, hintOffsetH}, 
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

void Client::setupAddress() {
  std::string ip;
  int port;
  bool hint = false;
  while (!setAddress(ip, port)) {
    ui.clearWindow();
    if (hint)
      showAddressHint();

    hint = true;
    
    auto [ip_str, port_str] = askAddress();
    try {
      port = std::stoi(port_str);
      ip = ip_str;
    } catch (...) {
      log << "User entered not a number. Asking him again...\n";
      continue;
    }
  }

  status = Status::connecting;
}

int Client::connectToHost() {
  return cstd::connect(
    socket.descriptor, 
    (cstd::sockaddr*)&(socket.address), 
    sizeof(socket.address)
  );
}

std::pair<std::string, std::string> Client::askAuthData() {
  const size_t 
    usernameOffsetV = 1,
    usernameOffsetH = 1,
    usernameBoxV = 1,
    usernameBoxH = 12;

  std::string username = ui.askForm(
    {usernameOffsetV, usernameOffsetH}, 
    {usernameBoxV, usernameBoxH},
     "Username: "
  );

  const size_t
    passwordOffsetV = 1,
    passwordOffsetH = 1,
    passwordBoxV = 1,
    passwordBoxH = 12;

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

bool Client::printMessage(size_t& space, size_t width, const std::string& message) {
  const size_t
    messageBoxTopPadding = 1,
    messageBoxLeftPadding = 1;

  std::stringstream stream(message);
  std::string buffer;
  while (std::getline(stream, buffer)) {
      size_t length = UserInterface::getTextRealSize(buffer);
      size_t height = ceil(length, width);

      if (space < height)
        return false;

      ui.print(
        {space - height + messageBoxLeftPadding, messageBoxTopPadding}, 
        {height, width}, 
        buffer
      );
      space -= height;
  }
  return true;
}

void Client::refreshMessages() {
  const size_t
    messageBoxTopPadding = 1,
    messageBoxBottomPadding = 1,
    chatBoxBottomPadding = 1,
    messageBoxLeftPadding = 1,
    messageBoxRightPadding = 1;

  size_t space = 
    ui.getWindowHeight()
    - chatBoxBottomPadding
    - chatspace
    - messageBoxBottomPadding
    - messageBoxTopPadding;

  size_t width = 
    ui.getWindowWidth()
    - messageBoxLeftPadding
    - messageBoxRightPadding;

  ui.clearSpace(
    {messageBoxTopPadding, messageBoxLeftPadding}, 
    {space, width}
  );

  auto message = data.head;

  while (data.isMessage(message)) {
    if (!printMessage(space, width, (*message).content)) {
      return;
    }

    ++message;
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
  const int
    kCommandCallback = 4,
    kHistoricMessage = 5;

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
      
      if (object.hasReturnCode() && object.code == 6) {
        data.clear();
        sendCommand("/scrollup");
      } else if (data.objects.empty()) {
        if (object.hasReturnCode() && object.code == kCommandCallback) {
          object.setId(-1);
        }
        data.insert(object);
        scroll = false;
        update.store(true);

      } else if (object.hasReturnCode() && object.code == kCommandCallback) {
        object.setPrev(data.objects.front().id);
        object.setId(data.objects.front().id);
        data.objects.push_front(object);

      } else if (object.id == data.objects.back().prev) {
        data.objects.push_back(object);
        scroll = false;

      } else if (object.prev == data.objects.front().id) {
        data.objects.push_front(object);

      } else if (data.objects.back().hasReturnCode() && data.objects.back().code == kCommandCallback && object.hasReturnCode() && object.code == kHistoricMessage) {
        data.objects.push_back(object);
        auto it = --data.objects.end();
        while ((*it).hasReturnCode() && (*it).code == kCommandCallback) {
          (*it).setPrev(data.objects.back().id);
          (*it).setId(data.objects.back().id);
          if (it == data.objects.begin()) {
            break;
          }
        }
      } else if (data.objects.front().hasReturnCode() && data.objects.front().code == kCommandCallback) {
        data.objects.push_front(object);
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

void ObjectTree::clear() {
  objects.clear();
  head = objects.begin();
}

ObjectTree::ObjectTree() {
  head = objects.begin();
}

bool ObjectTree::isMessage(std::list<Object>::iterator message) {
  return message != objects.end();
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
    "  >"
  );
}
