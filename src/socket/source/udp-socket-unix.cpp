#include "udp-socket.hpp"

#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

UdpSocket::~UdpSocket() {
  close(descriptor);
}

UdpSocket::UdpSocket(int port) {
  descriptor = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
  set_port(port);
}

void UdpSocket::set_port(int port) {
  address.addr.sin_family = domain;
  address.addr.sin_addr.s_addr = htonl(INADDR_ANY);
  address.addr.sin_port = htons(port);
}

int UdpSocket::bind() {
  return ::bind(
          descriptor,
          reinterpret_cast<sockaddr*>(&address.addr),
          sizeof(address.addr)
  );
}

std::string UdpSocket::get_ip_address() const {
  return {inet_ntoa(address.addr.sin_addr)};
}

char SocketBase::buffer[buffer_size];

#include <cstring> // memcpy
void UdpSocket::send(const std::string& message, Address& destination) const {
  static const int modulo = 127;
  buffer[0] = message.size() % modulo;
  buffer[1] = message.size() / modulo;

  memcpy(buffer + 2, message.c_str(), message.size());

  destination.len = sizeof(destination.addr);

  sendto(
          descriptor,
          buffer,
          2 + message.size(),
          0,
          reinterpret_cast<sockaddr*>(&destination.addr),
          destination.len
  );
}

std::pair<std::string, Address> UdpSocket::read() const {
  Address addr;
  int ok = recvfrom(
          descriptor,
          buffer,
          2,
          0,
          reinterpret_cast<sockaddr*>(&addr.addr),
          &addr.len
  );

  if (ok < 0 || ok > buffer_size) {
    return {"", addr};
  }

  static const int modulo = 127;
  size_t length = buffer[0] + buffer[1] * modulo;
  int bytes = recvfrom(
          descriptor,
          buffer,
          length,
          0,
          reinterpret_cast<sockaddr*>(&addr.addr),
          &addr.len
  );

  if (bytes < 0 || bytes > buffer_size) {
    return {"", addr};
  }

  std::string s;
  for (size_t i = 0; i < bytes; ++i) {
    s.push_back(buffer[i]);
  }
  return {s, addr};
}

void UdpSocket::get_peer_name(Address& addr) {
  socklen_t peer_address_size = sizeof(addr.addr);
  getpeername(
          descriptor,
          reinterpret_cast<sockaddr*>(&addr.addr),
          &peer_address_size
  );
}

int UdpSocket::get_port(Address& addr) const {
  return addr.addr.sin_port;
}

bool UdpSocket::setup_address(const std::string& ip_address) {
  return inet_pton(
          AF_INET,
          ip_address.c_str(),
          &(address.addr.sin_addr)
  );
}

int UdpSocket::get_descriptor() const {
  return descriptor;
}