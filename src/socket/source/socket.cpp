#include <socket.hpp>

void Socket::setAddress(int port) {
  address.sin_family = domain;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);
}

Socket::Socket(int port) {
  descriptor = cstd::socket(domain, SOCK_STREAM, 0);
  setAddress(port);
}

int Socket::bind() {
  return cstd::bind(descriptor, (struct cstd::sockaddr *)&address, sizeof(address));
}

int Socket::setSocketOption(int option, int value) {
  return cstd::setsockopt(descriptor, SOL_SOCKET, option, (char *)&value, sizeof(value));
}

int Socket::listen(int backlog) {
  return cstd::listen(descriptor, backlog);
}

Socket Socket::accept() {
  Socket peer;

  auto peerAddress = peer.getAddress();
  peer.descriptor = cstd::accept(descriptor, peerAddress.first, peerAddress.second);
  return peer;
}

std::string Socket::getIpAddress() {
  return std::string(cstd::inet_ntoa(address.sin_addr));
}

std::pair<cstd::sockaddr*, cstd::socklen_t*> Socket::getAddress() {
  static cstd::socklen_t addressLength = sizeof(address);
  return {reinterpret_cast<cstd::sockaddr*>(&address), &addressLength};
}

void Socket::send(std::string message) {
  cstd::send(descriptor, message.c_str(), message.length(), 0);
}

void Socket::send(char* buffer) {
  cstd::send(descriptor, buffer, sizeof(buffer), 0);
}

void Socket::getPeerName() {
  auto peerAddress = getAddress();
  cstd::getpeername(descriptor, peerAddress.first, peerAddress.second);
}

int Socket::getPort() {
  return address.sin_port;
}