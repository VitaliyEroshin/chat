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
    ipOffsetV = 1,
    ipOffsetH = 1,
    ipBoxV = 1,
    ipBoxH = 16;
  
  std::string ip = ui.askForm(
    {ipOffsetV, ipOffsetH}, 
    {ipBoxV, ipBoxH}, 
    "Enter ip address: "
  );

  const size_t
    portOffsetV = 1,
    portOffsetH = 1,
    portBoxV = 1,
    portBoxH = 5;

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
      log << "User entered not a number. Asking him again..." << std::endl;
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
    resultOffsetV = 4,
    resultOffsetH = 1;

  auto printResult = [this] (const std::string& message) {
    ui.print(
      {resultOffsetV, resultOffsetH},
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

  std::vector<std::string> lines;
  
  while (std::getline(stream, buffer)) {
    lines.push_back(buffer);
  }

  std::reverse(lines.begin(), lines.end());
  
  for (auto &line : lines) {
    size_t length = UserInterface::getTextRealSize(line);
    size_t height = ceil(length, width);

    if (space < height) return false;

    ui.print({space - height + messageBoxLeftPadding, messageBoxTopPadding},
             {height, width}, line);

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

void Client::delay(const std::string& label) {
  const size_t usecInMsec = 1000;

  cstd::usleep(config.get<int>(label) * usecInMsec);
}

void Client::showBackground(std::atomic<bool>& connecting) {
  const size_t
    loadingOffsetV = 1,
    loadingOffsetH = 1,
    loadingDots = 5;

  ui.clearWindow();
  int i = 0;

  while (connecting.load()) {
    ui.print(
      {loadingOffsetV, loadingOffsetH},
      "Loading" + std::string(i++ + 1, '.')
    );

    i %= loadingDots;
    delay("loadingBackgroundSpeed");
  }

  ui.clearWindow();
}

void Client::showConnectionVerdict(const std::string& verdict) {
  const size_t
    verdictOffsetV = 1,
    verdictOffsetH = 1;

  ui.print(
    {verdictOffsetV, verdictOffsetH},
    verdict
  );
}

int Client::connect() {
  setupAddress();
  std::atomic<bool> connecting(true);
  std::thread background(&Client::showBackground, this, std::ref(connecting));

  if (connectToHost() < 0) {
    delay("connectionBackgroundDuration");

    connecting.store(false);

    status = Client::Status::failed;
    
    showConnectionVerdict("Connection failed");
    delay("connectionFailedMessageDuration");
  }

  connecting.store(false);
  background.join();

  if (status == Client::Status::failed)
    return -1;

  status = Client::Status::authentification;
  showConnectionVerdict("Connected!");
  delay("connectionSucceedMessageDuration");
  ui.clearWindow();
  return 0;
}

void Client::refreshOutput() {
  while (run.load()) {
    if (update.load()) {
      refreshMessages();
      update.store(false);
    }
    delay("refreshOutputDelay");
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


void Client::parseTextObject(Object object) {
  const int
    kCommandCallback = 4,
    kHistoricMessage = 5,
    kChatSwitched = 6;

  bool scroll = (data.head == data.objects.begin());

  if (object.hasReturnCode(kChatSwitched)) {
    data.clear();
    sendCommand("/scrollup");

  } else if (data.objects.empty()) {
    data.insert(object);
    scroll = false;
    update.store(true);

  } else if (object.hasReturnCode(kCommandCallback)) {
    object.setPrev(data.frontId());
    object.setId(data.frontId());
    data.pushFront(object);

  } else if (object.id == data.objects.back().prev) {
    data.pushBack(object);
    scroll = false;

  } else if (object.prev == data.objects.front().id) {
    data.pushFront(object);

  } else if (data.objects.back().hasReturnCode(kCommandCallback) &&
             object.hasReturnCode(kHistoricMessage)) {
    data.objects.push_back(object);
    data.propagateIdFromBack();

  } else if (data.objects.front().hasReturnCode(kCommandCallback)) {
    data.objects.push_front(object);
  }

  update.store(true);
  
  if (scroll) {
    scrolldown();
  }
}

void Client::readServer() {
  while (run.load()) {
    std::string encoded = socket.read();

    if (encoded.empty()) {
      log << "Disconnected from the server :c" << std::endl;
      run.store(false);
      return;
    }

    Object object = encoder.decode(encoded);

    if (object.type == Object::Type::text)
      parseTextObject(object);

  }
}

void Client::quit() {
  run.store(false);
  socket.~Socket();
}

void Client::parseInputCommand(const std::string& command) {
  if (command == "/quit")
    quit();

  else if (command == "/refresh")
    initializeGUI();

  else if (command[0] == '/')
    sendCommand(command);
   
  else if (!command.empty())
    sendText(command);

}

void Client::readUserInput() {
  const size_t
    chatBottomPadding = 1,
    chatOffsetV = ui.getWindowHeight()
      - chatBottomPadding
      - chatspace,
    chatOffsetH = 4;

  chatspace = 1;
  while (run.load()) {
    drawChatPointer();
    std::string command = ui.input(
      {chatOffsetV, chatOffsetH},
      {chatspace, ui.getWindowWidth() - 2 * chatOffsetH}, 
      true
    );

    ui.clearSpace(
      {chatOffsetV, chatOffsetH}, 
      {chatspace, ui.getWindowWidth() - chatOffsetH}
    );

    chatspace = 1;

    parseInputCommand(command);
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

bool ObjectTree::isMessage(std::list<Object>::iterator message) const {
  return message != objects.end();
}

int ObjectTree::backId() const {
  if (!objects.back().hasId())
    return -1;

  return objects.back().id;
}

int ObjectTree::frontId() const {
  if (!objects.front().hasId())
    return -1;

  return objects.front().id;
}

void ObjectTree::pushFront(Object object) {
  objects.push_front(object);
}

void ObjectTree::pushBack(Object object) {
  objects.push_back(object);
}

void ObjectTree::propagateIdFromBack() {
  const size_t
    kCommandCallback = 4;

  auto it = --objects.end();
  while ((*it).hasReturnCode(kCommandCallback)) {
    (*it).setPrev(backId());
    (*it).setId(backId());

    if (it == objects.begin()) break;
  }
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
