#pragma once
#include <string>
#include <utility>
#include <iostream>

namespace cstd {
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <arpa/inet.h>
}

class Socket {
public:
  typedef cstd::sockaddr_in Address;

  const int domain = AF_INET;

  int descriptor;
  Address address;

  Socket(int port);
  Socket() = default;
  ~Socket();

  int bind();
  int setSocketOption(int option, int value);
  int listen(int backlog);
  void setAddress(int port);
  Socket accept();
  std::string getIpAddress();
  void send(std::string message);
  void send(char* buffer);
  void send(char* buffer, size_t length);
  void getPeerName();
  int getPort();

private:
  std::pair<cstd::sockaddr*, cstd::socklen_t*> getAddress();
};
