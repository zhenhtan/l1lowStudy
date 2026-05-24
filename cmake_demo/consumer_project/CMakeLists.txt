cmake_minimum_required(VERSION 3.16)
project(MathLibConsumer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use the math_lib installed under ~/.local
set(MATH_LIB_ROOT "$ENV{HOME}/.local")

add_executable(consumer_demo src/main.cpp)

# Headers installed as ~/.local/include/math/math.h
# so include root must be ~/.local/include
target_include_directories(consumer_demo PRIVATE
    "${MATH_LIB_ROOT}/include"
)

# Static library installed as ~/.local/lib/libmath_lib.a
target_link_libraries(consumer_demo PRIVATE
    "${MATH_LIB_ROOT}/lib/libmath_lib.a"
)
