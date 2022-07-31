#pragma once
#include "socket-base-unix.hpp"
#include <string>
#include <utility>

class UdpSocket: public SocketBase {
public:
  UdpSocket() = default;
  explicit UdpSocket(int port);
  ~UdpSocket();

  void set_port(int port);
  int bind();

  std::string get_ip_address(Address& addr) const;
  std::string get_ip_address();
  void send(const std::string& message, Address& destination) const;
  void send(const std::string& message);
  std::pair<std::string, Address> read() const;

  void get_peer_name(Address& addr);
  int get_port(Address& addr) const;
  int get_port();

  bool setup_address(const std::string& ip_address);
  int get_descriptor() const;
};