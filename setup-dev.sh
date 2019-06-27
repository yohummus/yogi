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
if $LINUX; then TRIPLETS=("x64-linux"); fi
if $MACOS; then TRIPLETS=("x64-osx"); fi
if $WINDOWS; then TRIPLETS=("x64-windows-static" "x86-windows-static"); fi

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

for PKG in "${PACKAGES[@]}"; do
    for TRIPLET in ${TRIPLETS[*]}; do
        PKG_AND_TRIPLET=$PKG:$TRIPLET
        if [[ $INSTALLED_PACKAGES == *"$PKG_AND_TRIPLET"* ]]; then
            echo "Package $PKG_AND_TRIPLET already installed."
        else
            echo "Installing package $PKG_AND_TRIPLET..."
            ./vcpkg install $PKG_AND_TRIPLET
        fi
    done
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

if $WINDOWS; then PYTHON_CMD=python; else PYTHON_CMD=python3; fi
if ! command -v $PYTHON_CMD >/dev/null 2>&1; then
    echo "Python 3 (command: $PYTHON_CMD) not found. Please install it."
    exit 1
fi

if ! command -v pip3 >/dev/null 2>&1; then
    echo "Python Package Index (command: pip3) not found. Please install it."
    exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake (command: cmake) not found. Please install it."
    exit 1
fi

# Install required Python libraries
pip3 install -r yogi-python/requirements.txt

# Create build directory and run cmake
function run_cmake() {
    TRIPLET=$1; shift
    CMAKE_CUSTOM_ARGS=$*
    
    CMAKE_ARGS=(
        ..
        -DVCPKG_TARGET_TRIPLET=$TRIPLET
        -DCMAKE_BUILD_TYPE=Debug
        -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
        "${CMAKE_CUSTOM_ARGS[@]}"
    )
    
    # Put quotes around each argument (if it is not an empty string)
    for i in "${!CMAKE_ARGS[@]}"; do
        [ -z "${CMAKE_ARGS[$i]}" ] || CMAKE_ARGS[$i]="\"${CMAKE_ARGS[$i]}\""
    done
    
    if $WINDOWS; then
        if [[ $TRIPLET == *"x64"* ]]; then BITS=64; else BITS=32; fi
        
        VS_VERSION="2019"
        VS_TOOLS_DIR="C:/Program Files (x86)/Microsoft Visual Studio/$VS_VERSION/Community/VC/Auxiliary/Build"
        VS_TOOLS="$VS_TOOLS_DIR/vcvars$BITS.bat"
        
        TERMINAL_SCRIPT="OPEN TERMINAL.bat"
        echo cmd /K \"$VS_TOOLS\" > "$TERMINAL_SCRIPT"
        
        TEMP_FILE=$(mktemp)
        echo \"$VS_TOOLS\" > $TEMP_FILE
        echo cmake "${CMAKE_ARGS[@]}" >> $TEMP_FILE
        echo exit >> $TEMP_FILE
        cmd /K < $TEMP_FILE
        rm $TEMP_FILE
    else
        eval cmake "${CMAKE_ARGS[@]}"
    fi
}

function prepare_build() {
    BUILD_DIR=$1; shift
    TRIPLET=$1; shift
    CMAKE_CUSTOM_ARGS=("$@")
    
    if [ -d $BUILD_DIR/ ]; then
        echo "Build directory $BUILD_DIR already exists. Not running cmake."
        return
    fi
    
    echo "Running cmake in $BUILD_DIR ..."
    mkdir $BUILD_DIR
    cd $BUILD_DIR
    run_cmake $TRIPLET $CMAKE_CUSTOM_ARGS
    cd ..
}

if $WINDOWS; then
    prepare_build ./build-x64 x64-windows-static -G "NMake Makefiles"
    prepare_build ./build-x86 x86-windows-static -G "NMake Makefiles"
    prepare_build ./build-vs x64-windows-static
else
    prepare_build ./build ${TRIPLETS[0]}
fi
