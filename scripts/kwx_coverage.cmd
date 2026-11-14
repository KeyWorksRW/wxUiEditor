@echo off
rem ============================================================================
rem Derives the C4 coverage matrix, the C5 ratchet set and the promotion backlog.
rem
rem Writes only:
rem     .private\kwxSupported.md
rem     .private\kwxBacklog.md
rem     .private\kwxLedger.md          (the marked Phase 3 block)
rem     .private\kwx_coverage_data\   (raw kwxgen inventory)
rem     src\generate\ffi_ui\coverage.json
rem
rem Inputs: kwxgen on PATH (or the sibling kwxFFI build), the six descriptors and
rem verified.json under src\generate\ffi_ui, and each language repository's
rem .private\samples_status.md. No state is hand-set; re-running is safe and
rem produces byte-identical output on the same day.
rem
rem Usage (from the repository root):
rem     scripts\kwx_coverage.cmd
rem ============================================================================

setlocal

pushd "%~dp0.." || exit /b 1

pwsh -NoProfile -ExecutionPolicy Bypass -File "%~dp0kwx_coverage.ps1" %*
set "exit_code=%errorlevel%"

popd
exit /b %exit_code%
