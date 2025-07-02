@echo off

if not exist python mkdir python
if not exist ruby mkdir ruby
if not exist perl mkdir perl
if not exist build mkdir build

setlocal

REM Default is to generate code from Release build. Pass "debug" to use Debug build.
set "BUILD_TYPE=Release"
if /i "%1"=="debug" set "BUILD_TYPE=Debug"
if /i "%1"=="release" set "BUILD_TYPE=Release"

echo Using build type: %BUILD_TYPE%
echo View results.log when this script finishes.

endlocal & set "BUILD_TYPE=%BUILD_TYPE%"

..\build\bin\%BUILD_TYPE%\wxUiEditor.exe --gen_coverage codegen_test.wxui
copy /Y codegen_test.log results.log >nul

echo. >> results.log
echo -------------- C++ code -------------- >> results.log

echo Running CMake build... >> results.log
cmake --build build --config Release --target check_build >> results.log 2>&1

echo. >> results.log
echo -------------- wxPython -------------- >> results.log

cd python
for %f in (*.py) do (
    echo Processing %f >> ..\results.log
    python -m py_compile "%f" >> ..\results.log 2>&1
    if %errorlevel%==0 echo Syntax OK >> ..\results.log
)
cd ..

echo. >> results.log
echo -------------- wxRuby -------------- >> results.log

cd ruby
for %f in (*.rb) do (
    echo Processing %f >> ..\results.log
    ruby -c "%f" >> ..\results.log 2>&1
)
cd ..

echo. >> results.log
echo -------------- wxPerl -------------- >> results.log

cd perl
for %f in (*.pl) do (
    echo Processing %f >> ..\results.log
    perl -c "%f" >> ..\results.log 2>&1
)
cd ..

REM Check if first or second parameter is "code"
REM Open results.log in VS Code has been specified.
set "RUN_CODE="
if /i "%1"=="code" set "RUN_CODE=1"
if /i "%2"=="code" set "RUN_CODE=1"

if defined RUN_CODE (
    cd ..\src
    cmd /c code ..\codegen_test\results.log
    cd ..\codegen_test
)
