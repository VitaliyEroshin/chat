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

#ifdef unix
  #define in_addr_t cstd::in_addr_t
  #define htonl cstd::htonl
  #define htons cstd::htons
#endif

#undef SOCK_STREAM
const int SOCK_STREAM = 1;

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
  std::pair<cstd::sockaddr*, unsigned int*> getAddress();
};
