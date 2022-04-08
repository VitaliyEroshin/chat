set(LOGGER_LIB_DIR ${PROJECT_SOURCE_DIR}/src/logger)
file(GLOB LOGGER_LIB_SOURCES ${LOGGER_LIB_DIR}/*cpp)
add_library(logger_lib ${LOGGER_LIB_SOURCES})

target_include_directories(logger_lib PUBLIC ${LOGGER_LIB_DIR})