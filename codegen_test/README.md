# DEPRECATED

With the change to using CMake's FetchContent instead of maintaining our own copy of the wxWidgets source code, this sub-directory can no longer be built. The only reason it is still here is because if we ever change to using wxFetch as a sub-module, then this could be enabled again.

In the meantime, the best coverage is the wxUiTesting project where both wxWidgets 3.2 and 3.3 C++ code can be tested, along with Perl, Python, and Ruby code.

## Old Notes

This directory is used to verify that wxUiEditor generates C++ code that compiles correctly. The steps to check from this directory are:

1) cd ..
2) cmake --build build --config Release --target wxUiEditor
3) cd codegen_test
4) ..\build\bin\Release\wxUiEditor.exe --gen_cpp codegen_test.wxui
5) cmake --build build --config Release --target check_build
