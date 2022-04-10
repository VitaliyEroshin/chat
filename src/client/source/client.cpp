#include "client.hpp"

Client::Client(Encoder& encoder, fs::Config& config)
    : status(Status::offline), 
      config(config), 
      socket(Socket(config.get<int>("port"))), 
      ui(UserInterface()), 
      encoder(encoder) 
  {}

bool Client::setAddress(std::string ip, int port) {
  socket.setAddress(port);
  if (ip == "localhost") {
    ip = "127.0.0.1";
  }
  return cstd::inet_pton(AF_INET, ip.c_str(), &(socket.address.sin_addr));
}

void Client::setupAddress() {
  std::string ip = ui.askForm({1, 1}, {1, 16}, "Enter ip address: ");
  int port;
  try {
    port = std::stoi(ui.askForm({2, 1}, {1, 4}, "Enter the port: "));
  } catch (...) {

  }

  while (!setAddress(ip, port)) {
    ui.print({1, 1}, {3, 30}, "");
    ui.print({4, 4}, "IP or port invalid. Please, try again.");
    ip = ui.askForm({1, 1}, {1, 16}, "Enter ip address: ");
    
    try {
      port = std::stoi(ui.askForm({2, 1}, {1, 4}, "Enter the port: "));
    } catch (...) {
      
    }
  }
  status = Status::connecting;
}

int Client::connectToHost() {
  return cstd::connect(socket.descriptor, (cstd::sockaddr*)&(socket.address), sizeof(socket.address));
}

int Client::auth() {
  std::string username = ui.askForm({1, 1}, {1, 12}, "Username: ");
  std::string password = ui.askForm({2, 1}, {1, 12}, "Password: ");

  Object object;
  object.type = Object::Type::loginAttempt;
  object.content += username;
  object.content.push_back(1);
  object.content += password;

  socket.send(encoder.encode(object));

  std::string query = socket.read();

  object = encoder.decode(query);

  if (object.code == 0) {
    ui.print({4, 1}, "Logged in!");
  } else if (object.code == 1) {
    ui.print({4, 1}, "Created new user!");
  } else if (object.code == 2) {
    ui.print({4, 1}, "Wrong password");
    return -1;
  }

  return 0;
}

void Client::sendText(const std::string& text) {
  Object object;
  object.content = text;
  object.type = Object::Type::text;
  data.insert(object.content);
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
  ui.print({ui.out.window.height - 2, 2}, "> ");
}

void Client::refreshMessages() {
  ui.print({1, 1}, {ui.out.window.height - 3, ui.out.window.width - 2}, "");
  auto it = data.head;
  for (int i = 0; i < ui.out.window.height - 4; ++i) {
    ui.print({ui.out.window.height - 3 - i, 2}, (*it).content);
    if (it == data.objects.begin()) {
      break;
    }
    --it;
  }
}

int Client::session() {
  if (connect() < 0) {
    return -1;
  }

  if (auth() < 0) {
    return -1;
  }

  initializeGUI();
  listen();

  return 0;
}

int Client::connect() {
  setupAddress();
  std::atomic<bool> connecting(true);
  std::thread background(&Client::showBackground, this, std::ref(connecting));

  if (connectToHost() < 0) {
    cstd::sleep(2);
    connecting.store(false);
    cstd::usleep(500 * 1000);
    status = Client::Status::failed;
    ui.print({1, 1}, {1, 20}, "Connection failed");
    cstd::sleep(2);
  }

  connecting.store(false);
  background.join();
  if (status == Client::Status::failed) {
    return -1;
  }

  status = Client::Status::authentification;
  ui.print({1, 1}, {1, 20}, "Connected!");
  cstd::usleep(300 * 1000);
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

void Client::refreshOutput(std::atomic<bool>& update, std::atomic<bool>& run) {
  while (run.load()) {
    if (update.load()) {
      refreshMessages();
      update.store(false);
    }
    cstd::usleep(config.get<int>("refreshOutputDelay") * 1000);
  }
}

void Client::listen() {
  std::atomic<bool> run(true);
  std::atomic<bool> update(false);

  std::thread userInputThread(&Client::readUserInput, this, std::ref(update), std::ref(run));
  std::thread serverReadThread(&Client::readServer, this, std::ref(update), std::ref(run));
  std::thread refreshOutputThread(&Client::refreshOutput, this, std::ref(update), std::ref(run));

  userInputThread.join();
  serverReadThread.join();
  refreshOutputThread.join();
}

void Client::parseMessage(const std::string& message) {
  std::string temp;
  for (auto &c : message) {
    if (c == '\n' || temp.size() == ui.out.window.width - 8) {
      data.insert(temp);
      temp.clear();
    }
    if (c != '\n') {
      temp.push_back(c);
    }
  }
  if (!temp.empty()) {
    data.insert(temp);
  }
}

void Client::readServer(std::atomic<bool>& update, std::atomic<bool>& run) {
  while (run.load()) {
    std::string message = socket.read();

    if (message.length() == 0) {
      run.store(false);
      return;
    }

    Object object = encoder.decode(message);
    if (object.type == Object::Type::text) {
      parseMessage(object.content);
      update.store(true);
    }
  }
}

void Client::readUserInput(std::atomic<bool>& update, std::atomic<bool>& run) {
  while (run.load()) {
    std::string command = ui.input({ui.out.window.height - 2, 4}, {1, ui.out.window.width - 8});

    ui.print({ui.out.window.height - 2, 2}, {1, ui.out.window.width - 8}, "> ");
    if (command == "/quit") {
      ui.~UserInterface();
      run.store(false);
      socket.~Socket();
      return;
    }

    if (command == "/refresh") {
      initializeGUI();
    }

    if (command[0] == '/') {
      sendCommand(command);
      continue;
    }
    if (command.empty()) {
      continue;
    }

    sendText(command);
    update.store(true);
  }
}

void ObjectTree::insert(const std::string& text) {
  Object object;
  object.content = text;
  objects.insert(head, object);
}

ObjectTree::ObjectTree() {
  Object empty;
  objects.push_back(empty);
  head = objects.begin();
}