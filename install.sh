#!/bin/sh

set -e # exit on error of any command

for PROJECT in yogi-core yogi-cpp yogi-python yogi-hub yogi-javascript; do
    echo "===== Building $PROJECT ====="
    cd $PROJECT
    mkdir -p build
    cd build
    cmake ..
    make
    echo "===== Installing $PROJECT ====="
    sudo make install
    cd ../..
done

echo "===== All done ====="
