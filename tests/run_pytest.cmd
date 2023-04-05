@echo off

@REM This batch file assumes that you have installed both Python and wxPython, that they are in
@REM your $PATH and you are able to run wxPython programs from the command line. It also
@REM assumes that you have built a debug version of wxUiEditor.exe and that it is in the
@REM build\stage\bin\debug directory.

echo Generating python files...
..\build\stage\bin\debug\wxUiEditor.exe --gen_python python_tests.wxui
type python_tests.log
cd python
python mainframe.py
cd ..

