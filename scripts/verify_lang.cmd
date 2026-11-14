@echo off
rem ============================================================================
rem One command per language: generate, build, drive, bounded verdict.
rem
rem Usage (from anywhere):
rem     scripts\verify_lang.cmd <lang> [quick|probe]
rem
rem   <lang>   go | fortran | julia | luajit | rust | typescript
rem   target   quick (default) = quick.wxui, the two form nodes only
rem            probe           = tests\quick_probe\probe.wxui, the full content
rem
rem Steps, in order:
rem   1. <wxUiEditor>\bin\Release\wxUiEditor.exe <project> --gen_<lang>
rem   2. <repo>\build_quick.cmd      (target quick)
rem      <repo>\build_probe.cmd      (target probe)
rem   3. <repo>\tests\drive_quick.ps1 -Target <target> -SkipBuild
rem
rem Every step is appended to .private\logs\verify_<lang>_<target>.log.
rem The command prints that log's last 30 lines -- which end with the drive's
rem single "VERDICT PASS|FAIL" line -- and returns the first non-zero exit code.
rem The assertions themselves are defined once in .private\kwxDrive.md.
rem ============================================================================

setlocal

for %%I in ("%~dp0..") do set "ROOT=%%~fI"
set "BIN=%ROOT%\bin\Release\wxUiEditor.exe"
set "QUICK=%ROOT%\..\quick"

set "LANG=%~1"
set "TARGET=%~2"
if "%LANG%"=="" goto :usage
if "%TARGET%"=="" set "TARGET=quick"

if /I "%LANG%"=="go"         ( set "REPO=kwxGO"         & set "SWITCH=--gen_go" )
if /I "%LANG%"=="fortran"    ( set "REPO=kwxFortran"    & set "SWITCH=--gen_fortran" )
if /I "%LANG%"=="julia"      ( set "REPO=kwxJulia"      & set "SWITCH=--gen_julia" )
if /I "%LANG%"=="luajit"     ( set "REPO=kwxLuaJIT"     & set "SWITCH=--gen_luajit" )
if /I "%LANG%"=="rust"       ( set "REPO=kwxRust"       & set "SWITCH=--gen_rust" )
if /I "%LANG%"=="typescript" ( set "REPO=kwxTypeScript" & set "SWITCH=--gen_typescript" )
if not defined REPO goto :bad_lang

set "PROJECT=%QUICK%\quick.wxui"
set "PROJECT_DIR=%QUICK%"
if /I "%TARGET%"=="probe" (
    set "PROJECT=%QUICK%\tests\quick_probe\probe.wxui"
    set "PROJECT_DIR=%QUICK%\tests\quick_probe"
)
if /I not "%TARGET%"=="quick" if /I not "%TARGET%"=="probe" goto :bad_target

set "REPO_DIR=%QUICK%\..\%REPO%"
set "LOG=%ROOT%\.private\logs\verify_%LANG%_%TARGET%.log"
if not exist "%ROOT%\.private\logs" mkdir "%ROOT%\.private\logs"

if not exist "%BIN%" (
    echo ERROR: %BIN% not found -- build wxUiEditor first.
    exit /b 1
)
if not exist "%PROJECT%" (
    echo ERROR: %PROJECT% not found.
    exit /b 1
)

set "GEN_RC=0"
set "BUILD_RC=0"
set "DRIVE_RC=0"

echo verify_lang %LANG% %TARGET% > "%LOG%"
echo log: %LOG% >> "%LOG%"
echo. >> "%LOG%"

rem --- 1. generate -----------------------------------------------------------------
echo [1/3] generate  %LANG% %TARGET%
pushd "%PROJECT_DIR%"
"%BIN%" "%PROJECT%" %SWITCH% >> "%LOG%" 2>&1
set "GEN_RC=%errorlevel%"
popd
echo [1/3] generate  exit=%GEN_RC%
if not "%GEN_RC%"=="0" (
    pwsh -NoProfile -ExecutionPolicy Bypass -Command "$log = Get-Content -LiteralPath '%LOG%'; @($log | Select-Object -Last 19); 'VERDICT FAIL'"
    exit /b %GEN_RC%
)

rem --- 2. build --------------------------------------------------------------------
echo [2/3] build     %REPO% %TARGET%
call "%REPO_DIR%\build_%TARGET%.cmd" >> "%LOG%" 2>&1
set "BUILD_RC=%errorlevel%"
echo [2/3] build     exit=%BUILD_RC%
if not "%BUILD_RC%"=="0" (
    rem Bounded failure report: at most 19 error lines plus the verdict.
    pwsh -NoProfile -ExecutionPolicy Bypass -Command "$log = @(Get-Content -LiteralPath '%LOG%' | Where-Object { $_.Trim() -ne '' }); $errors = @($log | Where-Object { $_ -match 'error|:\d+:|expected|undefined|traceback|failed' } | Select-Object -First 19); if ($errors.Count -eq 0) { $errors = @($log | Select-Object -Last 19) }; $errors; 'VERDICT FAIL'"
    exit /b %BUILD_RC%
)

rem --- 3. drive --------------------------------------------------------------------
echo [3/3] drive     %REPO% %TARGET%
pwsh -NoProfile -ExecutionPolicy Bypass -File "%REPO_DIR%\tests\drive_quick.ps1" -Target %TARGET% -SkipBuild >> "%LOG%" 2>&1
set "DRIVE_RC=%errorlevel%"
echo [3/3] drive     exit=%DRIVE_RC%

:report
echo.
echo --- last 30 log lines (%LOG%) ---
rem The drive's single VERDICT line is the last line of its output. If it never
rem ran or crashed, emit the verdict here so the caller always sees one.
pwsh -NoProfile -ExecutionPolicy Bypass -Command "$log = Get-Content -LiteralPath '%LOG%' -Tail 30; $log; $last = ($log | Where-Object { $_.Trim() -ne '' } | Select-Object -Last 1); if ($last -notmatch '^VERDICT (PASS|FAIL)$') { 'VERDICT FAIL' }"
echo --- end of log ---

set "RC=%GEN_RC%"
if "%RC%"=="0" set "RC=%BUILD_RC%"
if "%RC%"=="0" set "RC=%DRIVE_RC%"
echo verify_lang %LANG% %TARGET%: generation=%GEN_RC% build=%BUILD_RC% drive=%DRIVE_RC% first-nonzero=%RC%
exit /b %RC%

:bad_lang
echo ERROR: unknown language "%LANG%".
echo        expected: go, fortran, julia, luajit, rust, typescript
exit /b 2

:bad_target
echo ERROR: unknown target "%TARGET%" ^(expected "quick" or "probe"^).
exit /b 2

:usage
echo Usage: verify_lang.cmd ^<lang^> [quick^|probe]
echo.
echo   lang    go ^| fortran ^| julia ^| luajit ^| rust ^| typescript
echo   target  quick (default) -- quick.wxui, the two form nodes only
echo           probe           -- tests\quick_probe\probe.wxui, the full content
echo.
echo   Generates, builds, drives, prints the last 30 log lines and returns the
echo   first non-zero exit code. Assertions: .private\kwxDrive.md
exit /b 2
