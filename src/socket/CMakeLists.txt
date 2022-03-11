set(SOCKET_LIB_DIR ${PROJECT_SOURCE_DIR}/src/socket)

add_library(sockets_lib ${SOCKET_LIB_DIR}/source/socket.cpp)

target_include_directories(sockets_lib PUBLIC ${SOCKET_LIB_DIR}/include)