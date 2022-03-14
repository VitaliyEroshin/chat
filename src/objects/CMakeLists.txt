set(OBJECTS_LIB_DIR ${PROJECT_SOURCE_DIR}/src/objects)
file(GLOB OBJECTS_LIB_SOURCES ${OBJECTS_LIB_DIR}/*cpp)
add_library(objects_lib ${OBJECTS_LIB_SOURCES})

target_include_directories(objects_lib PUBLIC ${OBJECTS_LIB_DIR})