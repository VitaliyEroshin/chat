include_directories(${GTEST_INCLUDE_DIRS})

add_executable(encoder_test encoder_test.cpp)

target_link_libraries(encoder_test objects_lib)
target_link_libraries(encoder_test ${GTEST_LIBRARIES} Threads::Threads)


target_include_directories(encoder_test PUBLIC ${OBJECTS_LIB_DIR})

set_target_properties(encoder_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests/bin
)