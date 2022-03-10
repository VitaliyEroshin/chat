cmake_minimum_required(VERSION 3.20)

project (
    Chat
    VERSION 1.0
    LANGUAGES CXX 
)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/socket)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/server)