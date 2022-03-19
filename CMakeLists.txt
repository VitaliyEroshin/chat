cmake_minimum_required(VERSION 3.10)

project (
    Chat
    VERSION 1.2
    LANGUAGES CXX 
)


set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/src/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/objects)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/socket)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/server)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/client)