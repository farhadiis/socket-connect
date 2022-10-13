cmake_minimum_required(VERSION 3.16)
project(socket-app
        VERSION 1.0.0
        DESCRIPTION "An implementation of socket and websocket protocol for mobile and web in one purpose")

configure_file(manifest.h.in manifest.h)

# flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSERVER_DEBUG")
set(CMAKE_CXX_FLAGS -pthread)

# target
file(GLOB_RECURSE sources "src/*.cpp")
include_directories(./src/foundation/include ./src/include)
add_executable(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

# 3rd party libs

# <------------ add openssl dependency -------------->
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL)

install(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT binaries)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")