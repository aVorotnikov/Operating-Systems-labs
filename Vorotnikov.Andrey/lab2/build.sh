#!/bin/bash

BUILD_DIR="build"
BIN_DIR="bin"

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
make
cd ..

[ ! -d $BIN_DIR ] && mkdir $BIN_DIR

CONNS=("fifo" "mq" "seg")
for CONN in "${CONNS[@]}"
do
    cp ${BUILD_DIR}/host_${CONN} $BIN_DIR
    cp ${BUILD_DIR}/client_${CONN} $BIN_DIR
done

rm -r $BUILD_DIR
