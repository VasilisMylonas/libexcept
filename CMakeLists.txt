cmake_minimum_required(VERSION 3.0.0)
project(libexcept VERSION 0.1.0)

option(LIBEXCEPT_THREAD_AWARE "Disable for single-threaded programs" ON)
option(LIBEXCEPT_SIGNAL_AWARE "Disable if not handling signals" ON)

add_library(except except.c)
if(LIBEXCEPT_THREAD_AWARE)
    target_compile_definitions(except PUBLIC LIBEXCEPT_THREAD_AWARE)
endif()
if(LIBEXCEPT_SIGNAL_AWARE)
    target_compile_definitions(except PUBLIC LIBEXCEPT_SIGNAL_AWARE)
endif()

add_executable(except_test except_test.c)
target_link_libraries(except_test except pthread)
add_test(NAME except_test COMMAND except_test)
enable_testing()
