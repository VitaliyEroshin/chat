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

Socket::~Socket() {
  cstd::close(descriptor);
}

void Socket::bind() {
  cstd::bind(descriptor, (struct cstd::sockaddr *)&address, sizeof(address));
}

void Socket::setSocketOption(int option, int value) {
  cstd::setsockopt(descriptor, SOL_SOCKET, option, (char *)&value, sizeof(value));
}

void Socket::listen(int backlog) {
  cstd::listen(descriptor, backlog);
}

Socket Socket::accept() {
  Socket peer;

  cstd::sockaddr* peerAddress = reinterpret_cast<cstd::sockaddr*>(&peer.address);
  size_t addressLength = sizeof(peer.address);
  cstd::socklen_t* peerAddressLength = reinterpret_cast<cstd::socklen_t*>(&addressLength);

  peer.descriptor = cstd::accept(descriptor, peerAddress, peerAddressLength);
}

std::string Socket::getIpAddress() {
  return std::string(cstd::inet_ntoa(address.sin_addr));
}