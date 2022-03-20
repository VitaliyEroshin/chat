#include "client.hpp"

int main() {
  StrEncoder encoder;
  Client client(encoder);
  client.session();
}