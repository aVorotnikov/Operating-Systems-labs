cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(deleter)

add_compile_options(-Wall -Werror)
add_executable(${PROJECT_NAME} main.cpp utils.cpp utils.h
        Deleter.cpp Deleter.h
        Daemon.cpp Daemon.h
        ConfigReader.cpp ConfigReader.h)
