#include "client.hpp"
#include "ui.hpp"

Client client;

void readUserInput(std::atomic<bool>& run) {
  while (run.load()) {
    std::string command = client.ui.input("> ");
    client.ui.clearPreviousLine();
    std::cout << command << "\n\r";
    if (command == "/quit") {
      run.store(false);
      client.socket.~Socket();
      return;
    }
    client.socket.send(command);
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
    
    std::cout << "\r" << message << "(" << readBytes << " bytes)";
    std::flush(std::cout);
    std::cout << "\n\r" << client.ui.inputInvite;
    std::cout << client.ui.buffer.left;
    std::flush(std::cout);
    for (auto &x : client.ui.buffer.right) {
      std::cout << x;
    }
    for (auto &x : client.ui.buffer.right) {
      client.ui.coursor.moveLeft();
    }

    std::flush(std::cout);
  }
}

void listen() {
  std::atomic<bool> run(true);
  std::thread userInputThread(readUserInput, std::ref(run));
  std::thread serverReadThread(readServer, std::ref(run));

  userInputThread.join();
  serverReadThread.join();
}

int session() {
  client.ui.initWindow();
  int rcode = client.connect();
  if (rcode < 0) {
    std::cout << "Connection failed. (" << rcode << ")\n\r";
    return 1;
  }
  std::cout << "Connected\n\r";
  listen();
  
  return 0;
}

int main() {
  system("stty raw");
  session();
  
  system("stty cooked");
}