@echo off

@REM This batch file assumes that you have already run the initial CMake using Ninja as the
@REM generator. This should have created a build directory with a build-Debug.ninja file in it.
@REM It also assumes that you have built a debug version of wxUiEditor.exe and that it is in
@REM the build\stage\bin\debug directory.

echo Generating C++ files...
..\build\stage\bin\debug\wxUiEditor.exe --gen_cpp python_tests.wxui
type python_tests.log
cd build
ninja -f build-Debug.ninja
cd ..
build\Debug\cpptest.exe
