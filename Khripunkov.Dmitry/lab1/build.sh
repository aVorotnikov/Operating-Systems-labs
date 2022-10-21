#!/bin/bash

set -euo pipefail
root_dir=$(dirname "${BASH_SOURCE[0]}")
pid_file=/run/mydaemon.pid

cd "$root_dir"
mkdir -p build

cd build
cmake ..
cmake --build .

cd ..
mv build/mydaemon mydaemon
rm -rf build

sudo touch $pid_file
sudo chmod 666 $pid_file
