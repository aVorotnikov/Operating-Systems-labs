cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

project("copying_daemon")
add_executable(copying_daemon "main.cpp" "copier/copier.cpp" "daemon/daemon.cpp")
