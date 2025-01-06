#!/bin/env bash
set -e

# Install dependencies based on OS
if [ "$(uname)" == "Linux" ]; then
    yum update -y
    yum install -y cmake pkgconfig libusbx-devel libgusb-devel
elif [ "$(uname)" == "Darwin" ]; then
    brew install cmake pkg-config libusb
elif [ "$(uname)" == "MINGW"* ] || [ "$(uname)" == "MSYS"* ]; then
    vcpkg install libusb:x64-windows
fi

# Check if libcaer directory already exists
if [ ! -d "libcaer" ]; then
    # Clone and build libcaer
    git clone https://gitlab.com/inivation/dv/libcaer.git
    cd libcaer
    if [ "$(uname)" == "Darwin" ]; then
        cmake -S . -B build -DENABLE_OPENCV=0 -DENABLE_SERIALDEV=0 -DCMAKE_OSX_ARCHITECTURES=x86_64
    else
        cmake -S . -B build -DENABLE_OPENCV=0 -DENABLE_SERIALDEV=0
    fi
else
    cd libcaer
fi

cmake --build build --config Release
cmake --install build 