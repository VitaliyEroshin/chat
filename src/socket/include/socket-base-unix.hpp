#pragma once
#include <netinet/in.h> // sockaddr
#include <sys/select.h> // fd_set
#include <cstddef> // size_t

struct Address {
    sockaddr_in addr;
};

class SocketBase {
protected:
    const int domain = AF_INET;
    int descriptor;
    Address address;
    static const size_t buffer_size = 1024;
    static char buffer[buffer_size];
};

class DescriptorSetBase {
protected:
    fd_set descriptors;
};