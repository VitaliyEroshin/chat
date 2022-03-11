set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)

file(GLOB SERVER_SOURCES ${SERVER_DIR}/source/*.cpp)

add_executable(server ${SERVER_SOURCES})
target_include_directories(server PUBLIC ${SERVER_DIR}/include/)
target_include_directories(server PUBLIC ${SOCKET_LIB_DIR}/include/)

target_link_libraries(server PRIVATE sockets_lib)

set_target_properties(server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)