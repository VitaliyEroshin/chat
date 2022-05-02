add_subdirectory(${PROJECT_SOURCE_DIR}/tests/logger)
add_subdirectory(${PROJECT_SOURCE_DIR}/tests/encoder)

if (GITHUB_ACTIONS)
else (GITHUB_ACTIONS)
    enable_testing()
endif (GITHUB_ACTIONS)
    
add_test(encoder_test ${PROJECT_SOURCE_DIR}/tests/bin/encoder_test)
add_test(logger_test ${PROJECT_SOURCE_DIR}/tests/bin/logger_test)