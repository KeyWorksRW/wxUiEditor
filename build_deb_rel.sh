#!/bin/bash

mkdir -p build

cmake --build build --config Release

cd build
cpack -G DEB -C Release
ls -l *.deb
cd ..
