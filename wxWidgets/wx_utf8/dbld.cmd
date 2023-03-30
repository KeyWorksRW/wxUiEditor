@echo off

cd build
ninja -f build-Debug.ninja
dir /K Debug
cd ..
