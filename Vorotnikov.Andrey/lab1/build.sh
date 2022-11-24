#!bin/bash

BUILD_DIR="build"
EXECUTABLE_NAME="copying_daemon"

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
make
cd ..
cp $BUILD_DIR/$EXECUTABLE_NAME .
rm -r $BUILD_DIR
