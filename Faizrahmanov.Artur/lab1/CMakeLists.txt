cmake_minimum_required(VERSION 3.8)

project(FileMover)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

set(PROJECT_SOURCES
        main.cpp
        FileMoverDaemon.cpp FileMoverDaemon.h
        Config.cpp Config.h
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES})