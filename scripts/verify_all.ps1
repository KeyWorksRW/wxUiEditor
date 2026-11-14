<#
.SYNOPSIS
    Runs the kwx verification gate for several languages and prints one table.

.DESCRIPTION
    Loops scripts\verify_lang.cmd (generate -> build -> drive) and emits one pipe-delimited
    row per language plus the first non-zero exit code. The assertions themselves are defined
    once in .private\kwxDrive.md; this script adds none and fixes nothing.

    Languages run sequentially and fast-tier first (typescript, luajit, julia need no compile),
    so a cheap failure is visible before the compiled languages start. Each language's gate log
    is .private\logs\verify_<lang>_<target>.log; the concatenation is
    .private\logs\verify_all_<target>.log.

.PARAMETER Target
    quick (default) or probe.

.PARAMETER Languages
    Any subset of typescript, luajit, julia, go, fortran, rust. Default: all six.

.EXAMPLE
    pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_all.ps1

.EXAMPLE
    pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\verify_all.ps1 -Target probe -Languages go,rust
#>
[CmdletBinding()]
param(
    [ValidateSet('quick', 'probe')][string]$Target = 'quick',
    [string[]]$Languages
)

$ErrorActionPreference = 'Continue'
$root = Split-Path -Parent $PSScriptRoot
$order = @('typescript', 'luajit', 'julia', 'go', 'fortran', 'rust')

if (-not $Languages -or $Languages.Count -eq 0) {
    $Languages = $order
}
foreach ($lang in $Languages) {
    if ($order -notcontains $lang) {
        Write-Output "ERROR unknown language '$lang' (expected: $($order -join ', '))"
        exit 2
    }
}

$verify = Join-Path $PSScriptRoot 'verify_lang.cmd'
if (-not (Test-Path -LiteralPath $verify)) {
    Write-Output "ERROR $verify not found."
    exit 2
}

$logDir = Join-Path $root '.private\logs'
if (-not (Test-Path -LiteralPath $logDir)) {
    New-Item -ItemType Directory -Path $logDir -Force | Out-Null
}
$allLog = Join-Path $logDir "verify_all_$Target.log"
Set-Content -LiteralPath $allLog -Value "verify_all $Target $($Languages -join ' ')" -Encoding utf8

function Get-Stage([string]$text, [string]$stage) {
    $match = [regex]::Match($text, "(?m)^\[$stage\]\s+\S+\s+exit=(-?\d+)")
    if ($match.Success) { return $match.Groups[1].Value }
    return '-'
}

function Get-Assertions([string]$text) {
    $parts = @()
    foreach ($id in @('A1', 'A2', 'A3', 'A4', 'A5', 'A6', 'A7')) {
        $match = [regex]::Match($text, "(?m)^\S+\s+$id\s+\S+\s+(PASS|FAIL|SKIP)")
        if ($match.Success) {
            $parts += "$id=$($match.Groups[1].Value.Substring(0, 1))"
        }
        else {
            $parts += "$id=-"
        }
    }
    return ($parts -join ' ')
}

function Get-FirstError([string]$text, [string]$build, [string]$drive) {
    $match = $null
    if ($build -ne '0' -and $build -ne '-') {
        $match = [regex]::Match($text, '(?im)^.*(error|undefined symbol|expected|traceback|cannot find|not found).*$')
    }
    elseif ($drive -ne '0' -and $drive -ne '-') {
        $match = [regex]::Match($text, '(?m)^\S+\s+A\d\s+\S+\s+FAIL\s+.*$')
    }
    elseif ($build -eq '0' -and $drive -eq '0') {
        return '-'
    }
    else {
        $match = [regex]::Match($text, '(?im)^.*(error|fatal|exception).*$')
    }
    if ($null -eq $match -or -not $match.Success) { return '-' }
    $line = $match.Value.Trim() -replace '\|', '!'
    if ($line.Length -gt 120) { $line = $line.Substring(0, 120) + '…' }
    return $line
}

$started = Get-Date
$firstNonZero = 0
$failed = 0
$rows = @()

foreach ($lang in $Languages) {
    Write-Host "[verify_all] $lang $Target ..." -NoNewline
    $text = (& $verify $lang $Target 2>&1 | Out-String)
    $rc = $LASTEXITCODE

    Add-Content -LiteralPath $allLog -Value "===== $lang exit=$rc =====" -Encoding utf8
    Add-Content -LiteralPath $allLog -Value $text -Encoding utf8

    $gen = Get-Stage $text '1/3'
    $build = Get-Stage $text '2/3'
    $drive = Get-Stage $text '3/3'

    $verdict = 'FAIL'
    $vMatch = [regex]::Match($text, '(?m)^VERDICT (PASS|FAIL)\s*$')
    if ($vMatch.Success) { $verdict = $vMatch.Groups[1].Value }
    if ($rc -ne 0 -and $verdict -eq 'PASS') { $verdict = 'FAIL' }

    $assertions = Get-Assertions $text
    $errorLine = Get-FirstError $text $build $drive

    if ($verdict -ne 'PASS') { $failed++ }
    if ($rc -ne 0 -and $firstNonZero -eq 0) { $firstNonZero = $rc }

    $rows += "$lang|$gen|$build|$drive|$verdict|$assertions|$errorLine"
    Write-Host (" {0} (generate={1} build={2} drive={3})" -f $verdict, $gen, $build, $drive)
}

$elapsed = [int]((Get-Date) - $started).TotalSeconds
Write-Output "ELAPSED ${elapsed}s"
Write-Output "FIRST_NONZERO=$firstNonZero"
if ($failed -eq 0) {
    Write-Output "VERDICT ALL PASS ($($Languages.Count)/$($Languages.Count))"
}
else {
    Write-Output "VERDICT $failed/$($Languages.Count) FAIL"
}
Write-Output "LOG $(Join-Path $logDir "verify_all_$Target.log")"
Write-Output 'TABLE'
Write-Output 'language|gen|build|drive|verdict|assertions|first-error'
foreach ($row in $rows) { Write-Output $row }

exit $firstNonZero
