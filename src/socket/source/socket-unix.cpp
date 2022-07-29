#include "socket-base-unix.hpp"
#include "socket.hpp" // Unix system socket interface implementation

#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

Socket::~Socket() {
  close(descriptor);
}

Socket::Socket(int port) {
  descriptor = socket(domain, SOCK_STREAM, 0);
  set_port(port);
}

void Socket::set_port(int port) {
  address.addr.sin_family = domain;
  address.addr.sin_addr.s_addr = htonl(INADDR_ANY);
  address.addr.sin_port = htons(port);
}

int Socket::bind() {
  return ::bind(
    descriptor, 
    reinterpret_cast<sockaddr*>(&address.addr),
    sizeof(address.addr)
  );
}

int Socket::set_socket_option(int option, char value) {
  return setsockopt(
    descriptor, 
    SOL_SOCKET, 
    option, 
    &value, 
    sizeof(value)
  );
}

int Socket::listen(int backlog) {
  return ::listen(descriptor, backlog);
}

Socket Socket::accept() {
  Socket peer;
  auto* peer_address = &peer.address.addr;
  socklen_t peer_address_size = sizeof(*peer_address);
  peer.descriptor = ::accept(
    descriptor,
    reinterpret_cast<sockaddr*>(peer_address),
    &peer_address_size
  );

  return peer;
}

std::string Socket::get_ip_address() const {
  return {inet_ntoa(address.addr.sin_addr)};
}

void Socket::send(const std::string& message) const {
  static const int modulo = 127;
  buffer[0] = message.size() % modulo;
  buffer[1] = message.size() / modulo;

  ::send(descriptor, buffer, 2, 0);
  ::send(descriptor, message.c_str(), message.size(), 0);
}

char SocketBase::buffer[buffer_size];

std::string Socket::read() const {
  int ok = ::read(descriptor, buffer, 2);
  
  if (ok < 0 || ok > buffer_size) {
    return "";
  }

  static const int modulo = 127;
  size_t length = buffer[0] + buffer[1] * modulo;
  int bytes = ::read(descriptor, buffer, length);
  
  if (bytes < 0 || bytes > buffer_size) {
    return "";
  }
  
  std::string s;
  for (size_t i = 0; i < bytes; ++i) {
    s.push_back(buffer[i]);
  }
  return s;
}

void Socket::get_peer_name() {
  socklen_t peer_address_size = sizeof(address.addr);
  getpeername( 
    descriptor, 
    reinterpret_cast<sockaddr*>(&address.addr), 
    &peer_address_size
  );
}

int Socket::get_port() const {
  return address.addr.sin_port;
}

void DescriptorSet::set(int descriptor) {
  FD_SET(descriptor, &descriptors);
}

void DescriptorSet::clear() {
  FD_ZERO(&descriptors);
}

bool DescriptorSet::count(int descriptor) {
  return FD_ISSET(descriptor, &descriptors);
}

void DescriptorSet::select(int max_descriptor) {
  ::select(max_descriptor + 1, &descriptors, nullptr, nullptr, nullptr);
}

bool Socket::setup_address(const std::string& ip_address) {
  return inet_pton(
    AF_INET, 
    ip_address.c_str(),
    &(address.addr.sin_addr)
  );
}

int Socket::get_descriptor() const {
  return descriptor;
}

int Socket::connect() {
  return ::connect(
    descriptor,
    reinterpret_cast<sockaddr*>(&address.addr),
    sizeof(address.addr)
  );
}