# ① 最低 CMake 版本要求
cmake_minimum_required(VERSION 3.16)

# ② 项目名
project(CmakeDemo)

# ③ 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(GNUInstallDirs)

option(ENABLE_WARNINGS "Enable common compiler warnings" ON)

# ④ 把 math/ 目录下的代码编译成一个静态库
add_library(math_lib STATIC
    math/math.cpp
)

# ⑤ 让库对外暴露自己的头文件目录
target_include_directories(math_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	#        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ⑥ 生成可执行文件 demo
# 把 main.cpp 放到子目录后，PRIVATE 不会把头文件目录传给 demo，PUBLIC 才会传递。
add_executable(demo app/main.cpp)

# ⑦ demo 链接 math_lib
target_link_libraries(demo PRIVATE math_lib)


if(ENABLE_WARNINGS)
    target_compile_options(math_lib PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(math_test tests/math_test.cpp)
    target_link_libraries(math_test PRIVATE math_lib)

    if(ENABLE_WARNINGS)
        target_compile_options(math_test PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME math_test COMMAND math_test)
endif()

install(TARGETS math_lib demo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES math/math.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/math)
