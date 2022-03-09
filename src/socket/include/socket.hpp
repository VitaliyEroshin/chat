#pragma once
#include <string>
#include <utility>

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
  ~Socket() = default;

  int bind();
  int setSocketOption(int option, int value);
  int listen(int backlog);
  void setAddress(int port);
  Socket accept();
  std::string getIpAddress();
  void send(std::string message);
  void send(char* buffer);
  void getPeerName();
  int getPort();

private:
  std::pair<cstd::sockaddr*, cstd::socklen_t*> getAddress();
};
