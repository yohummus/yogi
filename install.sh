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
    npm
    phantomjs
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
    yogi-supervisor
    yogi-echoer
    tools/yogi-ping
)

MIN_RAM_REQUIRED_IN_MB=1024
SWAP_SIZE_IN_MB=1024

ROOT=$(dirname $(readlink -f $0))

function install_required_packages {
    echo
    echo "===== Installing required packages ====="
    local PACKAGE_LIST=""
    for PACKAGE in "${REQUIRED_PACKAGES[@]}"
    do
        PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
    done
    sudo apt-get install -y $PACKAGE_LIST
}

function install_nodejs_legacy {
    local LINK="/usr/bin/node"

    echo
    echo "===== Installing nodejs-legacy ====="
    if [ ! -f $LINK ]; then
        sudo apt-get install -y nodejs-legacy
    else
        echo "No need to install nodejs-legacy since $LINK already exists."
    fi
}

function install_newer_npm {
    echo
    echo "===== Installing newer version of npm ====="
    if [ $(npm --version | sed "s/^\([0-9]*\).*/\1/") -lt 4 ]; then
        sudo npm config set registry http://registry.npmjs.org/
        sudo npm install -g npm
    else
        echo "Sufficiently recent version of npm is already installed."
    fi
}

function make_swap_file {
    echo
    echo "===== Creating SWAP file ====="
    local MEM=$(grep MemTotal /proc/meminfo | awk '{print $2}')
    if [ $MEM -lt $(expr $MIN_RAM_REQUIRED_IN_MB \* 1024) ]; then
        if [ $(wc -l /proc/swaps | sed "s/ .*//") -gt 1 ]; then
            echo "There already seems to be an active SWAP file or partition."
        else
            local FILE=/tmp/swapfile.yogi-installer
            echo "Creating $SWAP_SIZE_IN_MB MB swap file $FILE..."
            dd if=/dev/zero of=$FILE bs=1024 count=$(expr $SWAP_SIZE_IN_MB \* 1024)
            chmod 0600 $FILE
            mkswap $FILE
            swapon $FILE
        fi
    else
        echo "No need for a SWAP file since the system seems to have enough RAM (>= $MIN_RAM_REQUIRED_IN_MB MB)."
    fi
}

function build_project {
    echo
    echo "===== Building $PROJECT ====="
    local PROJECT=$1
    cd $ROOT/$PROJECT
    mkdir -p build
    cd build
    cmake ..
    make
}

function install_project {
    echo
    echo "===== Installing $PROJECT ====="
    local PROJECT=$1
    cd $ROOT/$PROJECT/build
    sudo make install
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
install_nodejs_legacy
install_newer_npm
make_swap_file
build_and_install_all_projects

echo
echo "===== All done ====="
