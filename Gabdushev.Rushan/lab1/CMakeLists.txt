cmake_minimum_required(VERSION 3.0.0)
project(lab1_daemon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

add_executable(lab1_daemon main.cpp daemon.cpp)