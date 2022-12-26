#!/bin/bash

DIR_NAME="build_tmp"
EXE_NAME="lab1_daemon"
PID_PATH="/var/run/daemon_file_cleaner.pid"

sudo touch $PID_PATH; sudo chmod 0666 $PID_PATH

mkdir $DIR_NAME
cd $DIR_NAME
cmake ..; make
cd ..
cp $DIR_NAME/$EXE_NAME .
rm -r $DIR_NAME