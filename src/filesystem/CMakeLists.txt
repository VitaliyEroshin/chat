set(FILESYSTEM_LIB_DIR ${PROJECT_SOURCE_DIR}/src/filesystem)
file(GLOB FILESYSTEM_LIB_SOURCES ${FILESYSTEM_LIB_DIR}/*cpp)
add_library(filesystem_lib ${FILESYSTEM_LIB_SOURCES})

target_include_directories(filesystem_lib PUBLIC ${FILESYSTEM_LIB_DIR})