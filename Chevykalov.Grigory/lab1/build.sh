#!/bin/bash

DIR_NAME="tmp"
EXE_NAME="lab1_daemon"
PID_PATH="/var/run/daemon_reminder.pid"

sudo touch $PID_PATH; sudo chmod 0666 $PID_PATH

mkdir $DIR_NAME; cd $DIR_NAME
cmake ..; make
cd ..
cp $DIR_NAME/$EXE_NAME .
rm -r $DIR_NAME