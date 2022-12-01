#! /bin/bash

mkdir build
cd build

cmake ../
cmake --build ./
mv deleter ../

cd ../
rm -rf build/