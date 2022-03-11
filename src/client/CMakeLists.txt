set(CLIENT_DIR ${CMAKE_SOURCE_DIR}/src/client)

file(GLOB CLIENT_SOURCES ${CLIENT_DIR}/source/*.cpp)

add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PUBLIC ${CLIENT_DIR}/include/)
target_include_directories(client PUBLIC ${SOCKET_LIB_DIR}/include/)

target_link_libraries(client PRIVATE sockets_lib)

set_target_properties(client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)