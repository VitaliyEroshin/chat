set(SOCKET_LIB_DIR ${PROJECT_SOURCE_DIR}/src/socket)

file(GLOB SOCKET_SOURCES ${SOCKET_LIB_DIR}/source/*.cpp)

add_library(sockets_lib ${SOCKET_SOURCES})

target_include_directories(sockets_lib PUBLIC ${SOCKET_LIB_DIR}/include)

install(
    TARGETS sockets_lib
    RUNTIME DESTINATION exe/bin
    LIBRARY DESTINATION exe/lib
)