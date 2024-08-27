This directory is used to verify that wxUiEditor generates C++ code that compiles correctly. The steps to check from this directory are:

1) cd ..
2) cmake --build build --config Release --target wxUiEditor
3) cd codegen_test
4) ..\build\bin\Release\wxUiEditor.exe --gen_cpp codegen_test.wxui
5) cmake --build build --config Release --target check_build
