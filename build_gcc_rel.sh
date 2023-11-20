#!/bin/bash

# Create build directory
mkdir build
cd build

# Configure CMake to use clang and clang++
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..

# Build release version
cmake --build . --config Release

cd build
cpack -G DEB -C Release
ls *.deb
cd ..
