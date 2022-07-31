set(THREADPOOL_LIB_DIR ${PROJECT_SOURCE_DIR}/src/threadpool)
file(GLOB THREADPOOL_LIB_SOURCES ${THREADPOOL_LIB_DIR}/source/*cpp)
add_library(threadpool_lib ${THREADPOOL_LIB_SOURCES})

find_package (Threads REQUIRED)

target_include_directories(threadpool_lib PUBLIC ${THREADPOOL_LIB_DIR}/include)

install(
        TARGETS threadpool_lib
        RUNTIME DESTINATION exe/bin
        LIBRARY DESTINATION exe/lib
)