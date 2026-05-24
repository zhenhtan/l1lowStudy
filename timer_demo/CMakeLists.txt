cmake_minimum_required(VERSION 3.14)
project(timer_demo CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

# ── Step 1: build Timer10ms as a static library ───────────────────────────────
add_library(timer10ms_lib STATIC
    Timer10ms.cpp
)

target_include_directories(timer10ms_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(timer10ms_lib PUBLIC Threads::Threads)

# ── Step 2: build the executable and link against the library ─────────────────
add_executable(timer_demo
    main.cpp
)

target_link_libraries(timer_demo PRIVATE timer10ms_lib)
