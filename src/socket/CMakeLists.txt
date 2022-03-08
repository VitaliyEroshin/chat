target_include_directories(sockets ${SOCKET_LIB}/include)
add_library(sockets ${SOCKET_LIB}/source/socket.cpp)