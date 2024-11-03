@echo off

REM This batch file is designed to remove directories and files that are part of the upstream
REM repository, but are not used in wxUiEditor. It's not necessary to run this batch file,
REM though it does make the directory structure cleaner.

REM ------------------- lunasvg_3 -------------------

cd lunasvg_3

REM Remove the examples directory
if exist examples (
    rd /s /q examples
)

REM Remove the cmake directory
if exist cmake (
    rd /s /q cmake
)

REM Remove the .github directory
if exist .github (
    rd /s /q .github
)

REM Remove the subprojects directory
if exist subprojects (
    rd /s /q subprojects
)

REM ------------------- plutovg -------------------

cd plutovg

REM Remove the examples directory
if exist examples (
    rd /s /q examples
)

REM Remove the cmake directory
if exist cmake (
    rd /s /q cmake
)

REM Remove the .github directory
if exist .github (
    rd /s /q .github
)

REM Remove the smiley.png file if it exists
if exist smiley.png (
    del /q smiley.png
)

cd ../..
