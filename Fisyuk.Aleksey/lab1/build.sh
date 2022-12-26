#!/bin/bash
# Building daemon lab
pidPath="/var/run/lab1.pid"
[[ -f $pidPath ]] || sudo touch "$pidPath"
sudo chmod 666 "$pidPath"

cmake .
cmake --build .
rm CMakeCache.txt Makefile cmake_install.cmake
rm -r CMakeFiles/