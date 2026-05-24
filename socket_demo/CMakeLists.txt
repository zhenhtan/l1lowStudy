cmake_minimum_required(VERSION 3.16)
project(socket_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(socket_server server.cpp)
add_executable(socket_client client.cpp)

target_compile_options(socket_server PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(socket_client PRIVATE -Wall -Wextra -Wpedantic)
