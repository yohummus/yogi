#!/bin/bash
set -e

REQUIRED_PACKAGES=(
    cmake
    g++
    libboost-all-dev
    googletest
    libprotobuf-dev
    protobuf-compiler
    libqt5core5a
    libqt5network5
    libqt5websockets5-dev
    nodejs
    nodejs-legacy
    npm
    python3-protobuf
    python-protobuf
    python3-setuptools
    python-setuptools
    python3-future
    python-future
    python-typing
    python-enum34
)

PROJECTS=(
    yogi-core
    yogi-cpp
    yogi-python
    yogi-hub
    yogi-javascript
)

function install_required_packages {
    echo "===== Installing required packages ====="
    PACKAGE_LIST=""
    for PACKAGE in "${REQUIRED_PACKAGES[@]}"
    do
        PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
    done
    sudo apt-get install $PACKAGE_LIST
}

function install_newer_npm {
    echo "===== Installing newer version of npm ====="
    sudo npm install -g npm
}

function build_project {
    echo "===== Building $PROJECT ====="
    PROJECT=$1
    cd $PROJECT
    mkdir -p build
    cd build
    cmake ..
    make
    cd ../..
}

function install_project {
    echo "===== Installing $PROJECT ====="
    PROJECT=$1
    cd $PROJECT/build
    sudo make install
    cd ../..
}

function build_and_install_all_projects {
    for PROJECT in "${PROJECTS[@]}"
    do
        build_project $PROJECT
        install_project $PROJECT
    done
}

#==============================================================================
# MAIN CODE
#==============================================================================
install_required_packages
install_newer_npm
build_and_install_all_projects

echo "===== All done ====="
