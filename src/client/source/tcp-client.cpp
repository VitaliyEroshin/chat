#include "client.hpp"
#ifndef UDP
int Client::connect() {
  setup_address();
  std::atomic<bool> connecting(true);
  std::thread background(&Client::show_background, this, std::ref(connecting));

  if (connect_to_host() < 0) {
    delay("connectionBackgroundDuration");

    connecting.store(false);

    status = Client::Status::failed;

    show_connection_verdict("Connection failed");
    delay("connectionFailedMessageDuration");
  }

  connecting.store(false);
  background.join();

  if (status == Client::Status::failed)
    return -1;

  status = Client::Status::authentification;
  show_connection_verdict("Connected!");
  delay("connectionSucceedMessageDuration");
  ui.clear_cli_window();
  return 0;
}

int Client::connect_to_host() {
  return socket.connect();
}

int Client::auth() {
  auto [username, password] = ask_auth_data();
  auto attempt = make_auth_attempt(username, password);
  socket.send(encoder.encode(attempt));

  std::string query = socket.read();
  attempt = encoder.decode(query);

  return print_auth_results(attempt.code);
}

void Client::read_server() {
  while (run.load()) {
    std::string encoded = socket.read();

    if (encoded.empty()) {
      log << "Disconnected from the server :c" << std::endl;
      run.store(false);
      return;
    }

    Object object = encoder.decode(encoded);

    if (object.type == Object::Type::text)
        parse_text_object(object);

  }
}


#endif