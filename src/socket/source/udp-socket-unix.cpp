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

std::string UdpSocket::get_ip_address(Address& addr) const {
  return {inet_ntoa(addr.addr.sin_addr)};
}

std::string UdpSocket::get_ip_address() {
  return get_ip_address(address);
}

void UdpSocket::send(const std::string& message, Address& destination) const {
  destination.len = sizeof(destination.addr);
  sendto(
          descriptor,
          message.c_str(),
          message.size(),
          0,
          reinterpret_cast<sockaddr*>(&destination.addr),
          destination.len
  );
}

void UdpSocket::send(const std::string& message) {
  send(message, address);
}

std::pair<std::string, Address> UdpSocket::read() const {
  Address addr;
  size_t length = buffer_size;
  recvfrom(
          descriptor,
          buffer,
          length,
          0,
          reinterpret_cast<sockaddr*>(&addr.addr),
          &addr.len
  );

  std::string s = buffer;
  for (size_t i = 0; i < s.size(); ++i) {
    buffer[i] = 0;
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

int UdpSocket::get_port() {
  return get_port(address);
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