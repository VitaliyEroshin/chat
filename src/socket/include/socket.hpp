#pragma once
#include "socket-base-unix.hpp" // include if unix
#include <string>

class Socket: public SocketBase {
public:
  Socket() = default;
  explicit Socket(int port);
  ~Socket();

  void set_port(int port);
  int bind();
  int set_socket_option(int option, int value);
  int listen(int backlog);
  
  Socket accept();
  std::string get_ip_address() const;
  void send(const std::string& message) const;
  std::string read() const;
  void get_peer_name();
  int get_port() const;

  bool setup_address(const std::string& ip_address);
  int get_descriptor() const;
  int connect();
};

class DescriptorSet: public DescriptorSetBase {
public:
  void set(int descriptor);
  void clear();
  bool count(int descriptor);
  void select(int max_descriptor);
};