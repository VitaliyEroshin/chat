#include <algorithm>
#include "client.hpp"
#include "ui.hpp"

Client client;

void readUserInput(std::atomic<bool>& run) {
  while (run.load()) {
    std::string command = client.ui.input({client.ui.out.window.height - 2, 4}, {1, client.ui.out.window.width - 8});
    
    client.ui.print({client.ui.out.window.height - 2, 4}, {1, client.ui.out.window.width - 8}, "");
    if (command == "/quit") {
      run.store(false);
      client.socket.~Socket();
      return;
    }
    if (command == "") {
      continue;
    }
  
    client.sendText(command);
    client.refreshMessages();
  }
}

void readServer(std::atomic<bool>& run) {
  char buffer[1025];
  while (run.load()) {
    int readBytes = cstd::recv(client.socket.descriptor, buffer, 1024, 0);
    if (readBytes <= 0) {
      std::cout << "Disconnected\n\r";
      run.store(false);
      return;
    }

    std::string message;
    for (int i = 0; i < readBytes; ++i) {
      message.push_back(buffer[i]);
    }
    
    Object obj = client.encoder.decode(message);
    if (obj.type == Object::Type::text) {
      std::string temp = "";
      for (int i = 0; i < obj.message.size(); ++i) {
        temp.push_back(obj.message[i]);
        if ((i + 1) % (client.ui.out.window.width - 8) == 0) {
          client.data.insert(temp);
          temp = "";
        }
      }
      if (!temp.empty()) {
        client.data.insert(temp);
      }
      // client.data.insert(obj.message);
      client.refreshMessages();
    }
  }
}

void listen() {
  std::atomic<bool> run(true);
  std::thread userInputThread(readUserInput, std::ref(run));
  std::thread serverReadThread(readServer, std::ref(run));

  userInputThread.join();
  serverReadThread.join();
}

void showBackground(std::atomic<bool>& connecting) {
  client.ui.clearWindow();
  int i = 0;
  while (connecting.load()) {
    client.ui.print({1, 1}, {1, 20}, "Loading" + std::string(i++ + 1, '.'));
    i %= 5;
    cstd::usleep(300 * 1000);
  }
  client.ui.clearWindow();
}

int connect() {
  client.setupAddress();
  std::atomic<bool> connecting(true);
  std::thread background(showBackground, std::ref(connecting));
  
  if (client.connectToHost() < 0) {
    cstd::sleep(2);
    connecting.store(false);
    cstd::usleep(500 * 1000);
    client.status = Client::Status::failed;
    client.ui.print({1, 1}, {1, 20}, "Connection failed");
    cstd::sleep(2);
  }

  connecting.store(false);
  background.join();
  if (client.status == Client::Status::failed) {
    return -1;
  }

  client.status = Client::Status::authentification;
  client.ui.print({1, 1}, {1, 20}, "Connected!");
  cstd::usleep(300 * 1000);
  client.ui.clearWindow();
  return 0;
}

int session() {
  if (connect() < 0) {
    return -1;
  }
  
  if (client.auth() < 0) {
    return -1;
  }

  client.initializeGUI();
  listen();
  
  return 0;
}

int main() {
  session();
}