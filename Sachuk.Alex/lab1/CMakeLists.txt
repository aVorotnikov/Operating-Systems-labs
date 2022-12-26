cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

project("lab1_10")
add_executable(lab1_10 
    ConfigReader/configReader.cpp 
    Logger/logger.cpp 
    Daemon/daemon.cpp
    main.cpp
)
