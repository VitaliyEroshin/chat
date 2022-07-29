#include "client.hpp"
#ifdef UDP
int Client::connect() {
  setup_address();

  socket.send("ping");
  auto [query, addr] = socket.read();
  if (query == "ping") {
    status = Client::Status::authentification;
    show_connection_verdict("Connected!");
    delay("connectionSucceedMessageDuration");
    ui.clear_cli_window();
    return 0;
  } else {
    return -1;
  }
}

int Client::auth() {
  auto [username, password] = ask_auth_data();
  auto attempt = make_auth_attempt(username, password);
  socket.send(encoder.encode(attempt));

  auto[query, addr] = socket.read();
  attempt = encoder.decode(query);

  return print_auth_results(attempt.code);
}

void Client::read_server() {
  while (run.load()) {
    auto [encoded, addr] = socket.read();

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