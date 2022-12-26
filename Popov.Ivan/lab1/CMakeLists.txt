cmake_minimum_required(VERSION 3.0.0)
project(bk_copier_daemon)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

add_executable(bk_copier_daemon main.cpp daemon.cpp config.cpp)