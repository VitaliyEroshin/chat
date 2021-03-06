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

#ifdef __APPLE__
  #define select cstd::select
#endif

#undef SOCK_STREAM
const int SOCK_STREAM = 1;

class Socket {
private:
  static const size_t bufferSize = 1024;
  static char buffer[bufferSize];

public:
  typedef cstd::sockaddr_in Address;

  const int domain = AF_INET;

  int descriptor = 0;
  Address address{};

  explicit Socket(int port);
  Socket() = default;
  ~Socket();

  int bind();
  int setSocketOption(int option, int value);
  int listen(int backlog);
  void setAddress(int port);
  Socket accept();
  std::string getIpAddress() const;
  void send(const std::string& message) const;
  void send(char* buffer) const;
  void send(char* buffer, size_t length) const;
  std::string read() const;
  void getPeerName();
  int getPort() const;

private:
  std::pair<cstd::sockaddr*, unsigned int*> getAddress();
};

struct DescriptorSet {
  fd_set descriptors;
  void set(int descriptor);
  void clear();
  bool count(int descriptor);
  fd_set* reference();
};