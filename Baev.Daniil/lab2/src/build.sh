#!/bin/bash
mkdir build
cd build
cmake -S ../ -B ./
make

rm -r host*autogen

mv host* ../

cd ../
rm -r build