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

std::string Socket::getIpAddress() const {
  return {cstd::inet_ntoa(address.sin_addr)};
}

std::pair<cstd::sockaddr*, unsigned int*> Socket::getAddress() {
  static cstd::socklen_t addressLength = sizeof(address);
  return {reinterpret_cast<cstd::sockaddr*>(&address), &addressLength};
}

void Socket::send(const std::string& message) const {
  buffer[0] = message.size() % 256;
  buffer[1] = message.size() / 256;
  cstd::send(descriptor, buffer, 2, 0);
  cstd::send(descriptor, message.c_str(), message.size(), 0);
}

void Socket::send(char* buff) const {
  cstd::send(descriptor, buff, sizeof(buff), 0);
}

void Socket::send(char* buff, size_t length) const {
  cstd::send(descriptor, buff, length, 0);
}

char Socket::buffer[bufferSize];

std::string Socket::read() const {
  int ok = cstd::read(descriptor, buffer, 2);
  
  if (ok < 0 || ok > bufferSize) {
    return "";
  }
  size_t length = buffer[0] + buffer[1] * 256;

  int bytes = cstd::read(descriptor, buffer, length);
  
  if (bytes < 0 || bytes > bufferSize) {
    return "";
  }
  
  std::string s;
  for (size_t i = 0; i < bytes; ++i) {
    s.push_back(buffer[i]);
  }
  return s;
}

void Socket::getPeerName() {
  auto peerAddress = getAddress();
  cstd::getpeername(descriptor, peerAddress.first, peerAddress.second);
}

int Socket::getPort() const {
  return address.sin_port;
}

void DescriptorSet::set(int descriptor) {
  FD_SET(descriptor, reference());
}

void DescriptorSet::clear() {
  FD_ZERO(reference());
}

bool DescriptorSet::count(int descriptor) {
  return FD_ISSET(descriptor, reference());
}

fd_set* DescriptorSet::reference() {
  return &descriptors;
}