cmake_minimum_required(VERSION 3.16)
project(gdb_coredump_demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(SPLIT_DEBUG_SYMBOLS "Split debug symbols into a separate file" ON)

# Key flags for better core dump debugging.
# -g3: full debug symbols
# -O0: disable optimization for accurate stack/locals
# -fno-omit-frame-pointer: cleaner stack traces
# -Wall/-Wextra: keep code quality warnings
add_executable(crash_demo crash.c)
target_compile_options(crash_demo PRIVATE -g3 -O0 -fno-omit-frame-pointer -Wall -Wextra)

# Useful if you need symbol names from the executable in backtraces.
target_link_options(crash_demo PRIVATE -rdynamic)

if(SPLIT_DEBUG_SYMBOLS)
	find_program(CMAKE_OBJCOPY NAMES objcopy llvm-objcopy REQUIRED)
	find_program(CMAKE_STRIP NAMES strip llvm-strip REQUIRED)

	add_custom_command(TARGET crash_demo POST_BUILD
        # 第一步：把完整符号抽取成独立的 .debug 文件
		COMMAND ${CMAKE_COMMAND} -E echo "[split-debug] create $<TARGET_FILE:crash_demo>.debug"
		COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:crash_demo> $<TARGET_FILE:crash_demo>.debug
		COMMAND ${CMAKE_COMMAND} -E echo "[split-debug] strip runtime binary"
        # 第二步：剥离可执行文件中的调试符号
		COMMAND ${CMAKE_STRIP} --strip-debug --strip-unneeded $<TARGET_FILE:crash_demo>
		COMMAND ${CMAKE_COMMAND} -E echo "[split-debug] add gnu debuglink"
        # 第三步：在剥离符号的可执行文件中添加 GNU debuglink，指向独立的 .debug 文件
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE_NAME:crash_demo>.debug $<TARGET_FILE:crash_demo>
		VERBATIM
	)
endif()
