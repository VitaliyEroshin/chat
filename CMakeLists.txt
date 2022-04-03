cmake_minimum_required(VERSION 3.10)

project (
    Chat
    VERSION 1.3
    LANGUAGES CXX 
)

if (TEST)
    message("RUNNING IN TESTING MODE")
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
endif (TEST)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/src/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/objects)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/filesystem)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/socket)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/server)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/client)

if (TEST)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests/encoder)

    enable_testing()
    
    add_test(encoder_test ${PROJECT_SOURCE_DIR}/tests/bin/encoder_test)
endif (TEST)
