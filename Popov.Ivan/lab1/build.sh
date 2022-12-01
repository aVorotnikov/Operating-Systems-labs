#!/bin/bash

PID_FILE_PATH="/var/run/bk_copier_daemon.pid"

sudo touch "$PID_FILE_PATH"
sudo chmod 666 "$PID_FILE_PATH"

mkdir ./build
cd ./build
cmake ..
make
cd ..
cp build/bk_copier_daemon .
rm -r build