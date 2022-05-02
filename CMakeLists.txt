cmake_minimum_required(VERSION 3.10)

project (
    Chat
    VERSION 1.3
    LANGUAGES CXX 
)

set(CMAKE_CXX_STANDARD 17)

option(TEST "Is in testing mode" off)

include_directories(${PROJECT_SOURCE_DIR}/src/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/objects)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/logger)

if (TEST)
    message("Running test mode")

    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)

else (TEST)
    message("Running normal mode")
    add_subdirectory(${PROJECT_SOURCE_DIR}/src/filesystem)
    add_subdirectory(${PROJECT_SOURCE_DIR}/src/socket)
    add_subdirectory(${PROJECT_SOURCE_DIR}/src/server)
    add_subdirectory(${PROJECT_SOURCE_DIR}/src/client)
endif (TEST)