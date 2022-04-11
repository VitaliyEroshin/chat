#include "server.hpp"

void print(Object& obj) {
  int attr = obj.attributes;
  for (int i = 0; i < 8; ++i) {
    std::cout << attr % 2;
    attr /= 2;
  }
  std::cout << "|" <<  obj.content << "\n";
}

int main() {
  Logger log = {&std::cout};

  fs::Config config(log);
  config.load("./config/server.cfg");

  SmartStorage storage("./config/storage.cfg", log);
  StrEncoder encoder;

  Server server(config.get<int>("port"), storage, encoder, log);

  server.loop();

  // storage.addUser("aboba", "great_password");
  // storage.createChat(1);
  // Object obj;
  // obj.type = Object::Type::text;
  // obj.content = "Hello world!";
  // storage.addMessage(obj, encoder, 1);
  // cstd::sleep(2);
  // storage.addMessage(obj, encoder, 1);
  // storage.addMessage(obj, encoder, 1);
  // storage.addMessage(obj, encoder, 1);

  // obj = storage.getMessage(2, encoder);
  // print(obj);
  // std::cout << "Prev: " << obj.prev << "\n"; 
}