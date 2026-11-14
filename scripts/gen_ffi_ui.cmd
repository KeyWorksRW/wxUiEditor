@echo off
rem ============================================================================
rem Regenerates src/generate/ffi_ui/<lang>.json for the six kwx languages.
rem
rem Requires kwxgen on PATH (built from the sibling kwxFFI repository). The
rem descriptors are generated output -- never hand-edit them. Each run also
rem rewrites manifest.json so src/generate/writers/ffi_ui.cpp can detect a
rem descriptor that is stale relative to the kwxFFI headers.
rem
rem Usage (from the repository root):
rem     scripts\gen_ffi_ui.cmd
rem ============================================================================

setlocal

where kwxgen >nul 2>nul
if errorlevel 1 (
    echo Error: kwxgen was not found on PATH.
    echo        Build the sibling kwxFFI repository and add its bin\Release directory to PATH.
    exit /b 1
)

pushd "%~dp0.." || exit /b 1

for %%L in (fortran go julia lua rust typescript) do (
    echo Generating src/generate/ffi_ui/%%L.json
    kwxgen emit-ui --headers ../kwxFFI/include --defs ../kwxFFI/src/kwx_defs.cpp --lang %%L --verify --out src/generate/ffi_ui/%%L.json
    if errorlevel 1 (
        echo Error: kwxgen emit-ui failed for %%L
        popd
        exit /b 1
    )
)

pwsh -NoProfile -ExecutionPolicy Bypass -File "%~dp0gen_ffi_ui_manifest.ps1"
if errorlevel 1 (
    echo Error: failed to write src/generate/ffi_ui/manifest.json
    popd
    exit /b 1
)

echo Descriptors and manifest.json are up to date.
popd
exit /b 0
