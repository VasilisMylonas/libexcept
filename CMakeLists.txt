cmake_minimum_required(VERSION 3.0.0)
project(libexcept VERSION 0.1.0)

add_library(except except.c)

add_executable(except_test except_test.c)
target_link_libraries(except_test except)
add_test(NAME except_test COMMAND except_test)
enable_testing()
