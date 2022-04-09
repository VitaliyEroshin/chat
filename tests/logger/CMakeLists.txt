include_directories(${GTEST_INCLUDE_DIRS})

add_executable(logger_test logger_test.cpp)

target_link_libraries(logger_test logger_lib)
target_link_libraries(logger_test ${GTEST_LIBRARIES} Threads::Threads)


target_include_directories(logger_test PUBLIC ${LOGGER_LIB_DIR})

set_target_properties(logger_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/bin
)