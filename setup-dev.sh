#!/bin/bash
set -e

# Check where we run this script from
if [ $(dirname $0) != "." ]; then
    echo "This script has to be run the project's root directory."
    exit 1
fi

# Determine operating system
LINUX=false
MACOS=false
WINDOWS=false

case $(uname -s) in
    Linux*)  LINUX=true;;
    Darwin*) MACOS=true;;
    CYGWIN*) WIDNOWS=true;;
    MINGW*)  WINDOWS=true;;
esac

if ! $LINUX && ! $MACOS && ! $WINDOWS; then
    echo "Unsupported operating system."
    exit 1
fi

if $WINDOWS; then
    echo "Please use install.bat on Windows"
    exit 1
fi

# Fetch submodules
echo "Fetching submodules if necessary..."
git submodule init
git submodule update

# Bootstrap vcpkg
cd vcpkg

if [ -f vcpkg ]; then
    echo "Executable for vcpkg already exists."
else
    echo "Bootstrapping vcpkg..."
    ./bootstrap-vcpkg.sh
fi

# Install vcpkg packages
if $LINUX; then TRIPLET=x64-linux; fi
if $MACOS; then TRIPLET=x64-osx; fi

PACKAGES=(
    "gtest"
    "msgpack"
    "nlohmann-json"
    "openssl"
    "boost-algorithm"
    "boost-asio"
    "boost-beast"
    "boost-endian"
    "boost-uuid"
    "boost-functional"
    "boost-optional"
    "boost-program-options"
    "boost-filesystem"
    "boost-container"
)

INSTALLED_PACKAGES=$(./vcpkg list)

for PKG in ${PACKAGES[*]}; do
    PKG_AND_TRIPLET=$PKG:$TRIPLET
    if [[ $INSTALLED_PACKAGES == *"$PKG_AND_TRIPLET"* ]]; then
        echo "Package $PKG_AND_TRIPLET already installed."
    else
        echo "Installing package $PKG_AND_TRIPLET..."
        ./vcpkg install $PKG_AND_TRIPLET
    fi
done

cd ..

# Check that required programs are installed
if ! command -v dotnet >/dev/null 2>&1; then
    echo "Dotnet Core (command: dotnet) not found. Please install it."
    exit 1
fi

if ! command -v node >/dev/null 2>&1; then
    echo "NodeJS (command: node) not found. Please install it."
    exit 1
fi

if ! command -v npm >/dev/null 2>&1; then
    echo "Node Package Manager (command: npm) not found. Please install it."
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "Python 3 (command: python3) not found. Please install it."
    exit 1
fi

# Install required Python libraries
pip3 install -r yogi-python/requirements.txt

# Create build directory and run cmake
if [ -d ./build/ ]; then
    echo "Build directory ./build already exists. Not running cmake."
else
    mkdir ./build
    cd ./build
    
    echo "Running cmake..."
    cmake .. -DVCPKG_TARGET_TRIPLET=$TRIPLET -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
    
    echo "Building everything..."
    make -j4
    
    cd ..
fi
