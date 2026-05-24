cmake_minimum_required(VERSION 3.14)
project(concurrency_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
add_link_options(-fsanitize=thread)

add_executable(concurrency_demo main.cpp)

add_executable(deadlock_detector_demo
	deadlock_detector_demo.cpp
	deadlock_detector.cpp
)

add_executable(gdb_deadlock_100_threads
	gdb_deadlock_100_threads.cpp
)
