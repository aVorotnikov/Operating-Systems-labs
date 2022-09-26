cmake_minimum_required(VERSION 3.23)
project(lab1)

set(CMAKE_CXX_STANDARD 14)

add_executable(lab1 main.cpp)
target_compile_options(lab1 PRIVATE -Wall -Wextra -Wpedantic -Werror)
