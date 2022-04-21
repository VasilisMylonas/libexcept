cmake_minimum_required(VERSION 3.0.0)
project(libexcept VERSION 0.1.0)

option(LIBEXCEPT_THREAD_AWARE "Disable for single-threaded programs" ON)

add_library(except except.c)
if(LIBEXCEPT_THREAD_AWARE)
    target_compile_definitions(except LIBEXCEPT_THREAD_AWARE)
endif()

add_executable(except_test except_test.c)
target_link_libraries(except_test except pthread)
add_test(NAME except_test COMMAND except_test)
enable_testing()
