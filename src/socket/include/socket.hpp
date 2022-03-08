#pragma once
#include <string>

namespace cstd {
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <arpa/inet.h>
}

class Socket {
  typedef cstd::sockaddr_in Address;

  const int domain = AF_INET;

  int descriptor;
  Address address;

  
  Socket(int port);
  ~Socket();

  void bind();
  void setSocketOption(int option, int value);
  void listen(int backlog);
  void setAddress(int port);
  Socket accept();

  std::string getIpAddress();

private:
  Socket();
};
