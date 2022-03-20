#include "client.hpp"

StrEncoder encoder;
Client client(encoder);

int main() {
  client.session();
}