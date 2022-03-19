#include "client.hpp"

Client::Client()
  : status(Status::offline), socket(Socket(8888)), ui(UserInterface())  {

}

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
  struct timeval timeout;
  timeout.tv_sec = 7;
  timeout.tv_usec = 0;

  return cstd::connect(socket.descriptor, (cstd::sockaddr*)&(socket.address), sizeof(socket.address));
}

int Client::auth() {
  std::string username = ui.askForm({1, 1}, {1, 12}, "Username: ");
  std::string password = ui.askForm({2, 1}, {1, 12}, "Password: ");

  Object obj;
  obj.type = Object::Type::loginAttempt;
  obj.message += username;
  obj.message.push_back(1);
  obj.message += password;

  socket.send(encoder.encode(obj));

  char buffer[1025];
  int readBytes = cstd::recv(socket.descriptor, buffer, 1024, 0);
  if (readBytes <= 0) {
    ui.print({4, 1}, "Disconnected");
    return -1;
  }

  std::string query;
  for (size_t i = 0; i < readBytes; ++i) {
    query.push_back(buffer[i]);
  }

  obj = encoder.decode(query);

  if (obj.ret == 0) {
    ui.print({4, 1}, "Logged in!");
  } else if (obj.ret == 1) {
    ui.print({4, 1}, "Created new user!");
  } else if (obj.ret == 2) {
    ui.print({4, 1}, "Wrong password");
    return -1;
  }

  return 0;
}

void Client::sendText(const std::string& text) {
  Object obj;
  obj.message = text;
  obj.type = Object::Type::text;
  obj.id = 1555;
  std::string en = encoder.encode(obj);
  data.insert(obj.message);
  socket.send(en);
}

void Client::initializeGUI() {
  ui.clearWindow();
  ui.print({ui.out.window.height - 2, 2}, "> ");
}

void Client::refreshMessages() {
  ui.log(1, "CRSR(" + std::to_string(ui.cursor.position.x) + ";" + std::to_string(ui.cursor.position.y) + ")");
  ui.print({1, 1}, {ui.out.window.height - 3, ui.out.window.width - 2}, "");
  auto it = data.head;
  for (int i = 0; i < ui.out.window.height - 4; ++i) {
    ui.print({ui.out.window.height - 3 - i, 2}, (*it).message);
    if (it == data.objects.begin()) {
      break;
    }
    --it;
        
  }
  ui.log(1, "CRSR(" + std::to_string(ui.cursor.position.x) + ";" + std::to_string(ui.cursor.position.y) + ")");
}

void ObjectTree::insert(const std::string& text) {
  Object obj;
  obj.message = text;
  if (objects.size() == 0) {
    head = objects.begin();
    objects.insert(head, obj);

    return;
  }
  objects.insert(head, obj);
}