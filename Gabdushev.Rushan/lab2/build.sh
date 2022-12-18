#!/bin/bash

mkdir tmp; cd tmp

cmake -S ../ -B ./; make

rm -rf host*autogen; mv host* ../

cd ../; rm -r tmp