#!/bin/bash

mkdir -p build

cmake --build build --config Release

cd build
cpack -G RPM -C Release
ls -l *.rpm
cd ..
