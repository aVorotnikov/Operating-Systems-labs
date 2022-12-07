#!/bin/bash

mkdir tmp; cd tmp

cmake -S ../ -B ./; make

rm -rf client*autogen; mv client* ../
rm -rf host*autogen; mv host* ../

cd ../; rm -r tmp