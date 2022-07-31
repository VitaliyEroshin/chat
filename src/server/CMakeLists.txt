set(SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)

file(GLOB SERVER_SOURCES ${SERVER_DIR}/source/*.cpp)
file(GLOB STORAGE_SOURCES ${SERVER_DIR}/source/storage/*.cpp)

add_executable(server ${SERVER_SOURCES} ${STORAGE_SOURCES})
target_include_directories(server PUBLIC ${SERVER_DIR}/include/)
target_include_directories(server PUBLIC ${SOCKET_LIB_DIR}/include/)
target_include_directories(server PUBLIC ${OBJECTS_LIB_DIR})
target_include_directories(server PUBLIC ${FILESYSTEM_LIB_DIR})
target_include_directories(server PUBLIC ${LOGGER_LIB_DIR})

target_link_libraries(server PRIVATE sockets_lib)
target_link_libraries(server PRIVATE objects_lib)
target_link_libraries(server PRIVATE filesystem_lib)
target_link_libraries(server PRIVATE logger_lib)
target_link_libraries(server PRIVATE threadpool_lib)

set_target_properties(server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

install(
    TARGETS server
    RUNTIME DESTINATION exe/bin
    LIBRARY DESTINATION exe/lib
)