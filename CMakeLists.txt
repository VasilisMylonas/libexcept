cmake_minimum_required(VERSION 3.0.0)
project(libexcept VERSION 0.1.0)

option(LIBEXCEPT_THREAD_AWARE "Disable for single-threaded programs" ON)

if(WIN32)
option(LIBEXCEPT_SIGNAL_AWARE "Disable if not handling signals" OFF)
else()
option(LIBEXCEPT_SIGNAL_AWARE "Disable if not handling signals" ON)
endif()

set(LIBEXCEPT_SJLJ ON)

add_library(except except.c)
configure_file(config.h.in config.h @ONLY)
target_include_directories(except PUBLIC ${CMAKE_BINARY_DIR})

add_executable(except_test except_test.c)
target_link_libraries(except_test except pthread)
add_test(NAME except_test COMMAND except_test)
enable_testing()
