cmake_minimum_required(VERSION 3.16)
project(MathLibConsumer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Search installed package in ~/.local by default.
list(PREPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")

find_package(MathLib 1.0.0 REQUIRED CONFIG)

add_executable(consumer_demo src/main.cpp)

target_link_libraries(consumer_demo PRIVATE
    MathLib::math_lib
)
