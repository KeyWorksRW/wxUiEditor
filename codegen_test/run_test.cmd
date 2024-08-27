@echo off

..\build\bin\Release\wxUiEditor.exe --gen_cpp codegen_test.wxui
type codegen_test.log

cmake --build build --config Release --target check_build
