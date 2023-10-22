@echo off

@REM This batch file assumes that you have built a debug version of wxUiEditor.exe and that it
@REM is in the ..\..\build\stage\bin\debug directory.

echo Generating C++ files...
..\..\build\stage\bin\debug\wxUiEditor.exe --gen_cpp sdi_test.wxui
type sdi_test.log
cmake.exe --build build --config Debug --target sdi_test
build\Debug\sdi_test.exe
