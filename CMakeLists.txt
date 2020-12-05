cmake_minimum_required(VERSION 3.0.0)

project(string-stream-handler)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_subdirectory(deps/Asampl)
endif()

set(CMAKE_CXX_STANDARD 17)
file(GLOB HANDLER_SRCS src/*.cpp)

add_compile_options(-g)

add_library(string-stream-handler SHARED ${HANDLER_SRCS})
target_link_libraries(string-stream-handler PRIVATE asampl-interface)

install(TARGETS string-stream-handler LIBRARY DESTINATION handlers)
