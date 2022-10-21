#!bin/bash

BUILD_DIR="build"
BIN_DIR="bin"
BIN_FILE="lab1"

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
make
cd ..
[ ! -d $BIN_DIR ] && mkdir $BIN_DIR
cp $BUILD_DIR/$BIN_FILE $BIN_DIR
rm -rf $BUILD_DIR
