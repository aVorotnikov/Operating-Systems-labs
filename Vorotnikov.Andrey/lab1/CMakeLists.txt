cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project("copying_daemon")

# Добавьте источник в исполняемый файл этого проекта.
add_executable(copying_daemon "main.cpp" "copier/copier.cpp")
