# ============================================================================
# conformance.ps1 -- regenerate the conformance kit and sweep it across all six
#                    languages with `kwxgen check-ui`.
#
# One command, one report.  The kit is generated from the compiled generator
# library by `wxUiEditor --gen_conformance`, so the class list, each class's
# parent chain and each class's properties are never hand-authored
# (.private/kwxPlan.md Phase 14).
#
# Steps
#   1. <wxUiEditor> --gen_conformance <quick>/tests/conformance
#   2. per kit form: <wxUiEditor> <form>.wxui --dump_ui_ir <ir>/<class>.json
#   3. per kit form per language: kwxgen check-ui --lang L --ir <ir> --descriptor <L>.json
#
# Outputs (the only files written by this script)
#   * <quick>/tests/conformance/*.wxui and kit.json  (regenerated from the library)
#   * .private/kwxConformance.md                    (the sweep report)
#   * .private/kwx_conformance_data/ir/*.json       (per-form IR, for inspection)
#
# Exit codes
#   0  the sweep completed (gaps are data, not failure)
#   2  the kit could not be generated, or a generated form does not load
#   1  usage or environment error (binary, kwxgen, descriptors)
#
# Usage (from the repository root):
#     scripts\conformance.cmd
#
# Optional parameters (all default to the sibling repositories):
#     -KwxFfiRoot <dir>   kwxFFI checkout providing include/ and src/kwx_defs.cpp
#     -QuickRoot <dir>    the quick project checkout
#     -KitDir <dir>       override the kit directory
# ============================================================================

param(
    [string]$KwxFfiRoot = '',
    [string]$QuickRoot = '',
    [string]$KitDir = ''
)

$ErrorActionPreference = 'Stop'

$repo_root = Split-Path -Parent $PSScriptRoot
$sibling_root = Split-Path -Parent $repo_root

if ([string]::IsNullOrWhiteSpace($KwxFfiRoot)) {
    $KwxFfiRoot = Join-Path $sibling_root 'kwxFFI'
}
if ([string]::IsNullOrWhiteSpace($QuickRoot)) {
    $QuickRoot = Join-Path $sibling_root 'quick'
}
if ([string]::IsNullOrWhiteSpace($KitDir)) {
    $KitDir = Join-Path $QuickRoot 'tests/conformance'
}

$descriptor_dir = Join-Path $repo_root 'src/generate/ffi_ui'
$data_dir = Join-Path $repo_root '.private/kwx_conformance_data'
$ir_dir = Join-Path $data_dir 'ir'
$report_path = Join-Path $repo_root '.private/kwxConformance.md'

$languages = @('fortran', 'go', 'julia', 'lua', 'rust', 'typescript')

# ----------------------------------------------------------------------------
# Small helpers
# ----------------------------------------------------------------------------

function Write-Utf8File {
    param([string]$Path, [string]$Text)
    [System.IO.File]::WriteAllText($Path, $Text, [System.Text.UTF8Encoding]::new($false))
}

function Add-Line {
    param([System.Text.StringBuilder]$Builder, [string]$Text)
    $null = $Builder.Append($Text).Append("`n")
}

function Resolve-Binary {
    $candidates = @(
        (Join-Path $repo_root 'bin/Release/wxUiEditor.exe'),
        (Join-Path $repo_root 'build/bin/Release/wxUiEditor.exe'),
        (Join-Path $repo_root 'bin/Debug/wxUiEditor.exe'),
        (Join-Path $repo_root 'build/bin/Debug/wxUiEditor.exe')
    )
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }
    throw 'wxUiEditor.exe not found -- build it with: ninja -C build -f build-Release.ninja'
}

function Resolve-Kwxgen {
    $command = Get-Command kwxgen -ErrorAction SilentlyContinue
    if ($null -ne $command) { return $command.Source }

    $candidate = Join-Path $KwxFfiRoot 'bin/Release/kwxgen.exe'
    if (Test-Path -LiteralPath $candidate) { return $candidate }

    throw "kwxgen is not on PATH and $candidate does not exist"
}

# ----------------------------------------------------------------------------
# Inputs
# ----------------------------------------------------------------------------

$binary = Resolve-Binary
$kwxgen = Resolve-Kwxgen

foreach ($lang in $languages) {
    $descriptor = Join-Path $descriptor_dir ($lang + '.json')
    if (-not (Test-Path -LiteralPath $descriptor)) {
        throw "Missing descriptor: $descriptor (run scripts\gen_ffi_ui.cmd)"
    }
}

$coverage_path = Join-Path $descriptor_dir 'coverage.json'
if (-not (Test-Path -LiteralPath $coverage_path)) {
    throw "Missing $coverage_path (run scripts\kwx_coverage.cmd)"
}

# ----------------------------------------------------------------------------
# 1. Regenerate the kit from the generator library
# ----------------------------------------------------------------------------

& $binary --gen_conformance $KitDir 2>&1 | Out-Null
$gen_exit = $LASTEXITCODE
if ($gen_exit -ne 0) {
    Write-Output "conformance: --gen_conformance failed with exit code $gen_exit"
    exit 2
}

$kit_json_path = Join-Path $KitDir 'kit.json'
if (-not (Test-Path -LiteralPath $kit_json_path)) {
    Write-Output "conformance: the kit manifest was not written: $kit_json_path"
    exit 2
}

$kit = Get-Content -LiteralPath $kit_json_path -Raw | ConvertFrom-Json -AsHashtable
$kit_entries = @($kit['forms'])
$kit_omitted = @($kit['omitted'])

# ----------------------------------------------------------------------------
# 2/3. Dump every form's IR, then check every form against every language
# ----------------------------------------------------------------------------

$null = New-Item -ItemType Directory -Force -Path $ir_dir
Get-ChildItem -LiteralPath $ir_dir -File -ErrorAction SilentlyContinue | Remove-Item -Force

$results = [System.Collections.Generic.List[hashtable]]::new()
$generation_results = [System.Collections.Generic.List[hashtable]]::new()
$load_failures = [System.Collections.Generic.List[string]]::new()

$generation_switches = @{
    fortran    = '--gen_fortran'
    go         = '--gen_go'
    julia      = '--gen_julia'
    lua        = '--gen_luajit'
    rust       = '--gen_rust'
    typescript = '--gen_typescript'
}

# "class|role" -> SortedSet of languages that report MISSING
$gap_map = @{}
# "class|role" -> SortedSet of languages that resolve
$resolved_map = @{}

foreach ($entry in $kit_entries) {
    $class_name = [string]$entry['class']
    $form_path = Join-Path $KitDir ($class_name + '.wxui')
    $ir_path = Join-Path $ir_dir ($class_name + '.json')

    & $binary $form_path --dump_ui_ir $ir_path 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $ir_path)) {
        $load_failures.Add($class_name)
        $results.Add(@{ class = $class_name; loaded = $false; clean = @(); gaps = @() })
        continue
    }

    $clean_languages = [System.Collections.Generic.List[string]]::new()
    $form_gaps = [System.Collections.Generic.List[hashtable]]::new()

    foreach ($lang in $languages) {
        $descriptor = Join-Path $descriptor_dir ($lang + '.json')
        $output = @(& $kwxgen check-ui --lang $lang --ir $ir_path --descriptor $descriptor 2>&1)
        $exit_code = $LASTEXITCODE

        $missing = [System.Collections.Generic.List[hashtable]]::new()
        foreach ($line in $output) {
            $text = [string]$line
            $match = [regex]::Match($text, '^MISSING\s+(?<lang>\S+)\s+(?<class>\S+)\s+(?<role>\S+)\s*$')
            if ($match.Success) {
                $missing.Add(@{
                    lang  = $match.Groups['lang'].Value
                    class = $match.Groups['class'].Value
                    role  = $match.Groups['role'].Value
                })
            }
        }

        if ($exit_code -eq 0 -and $missing.Count -eq 0) {
            $clean_languages.Add($lang)
            continue
        }

        foreach ($gap in $missing) {
            $key = $gap.class + '|' + $gap.role
            if (-not $gap_map.ContainsKey($key)) {
                $gap_map[$key] = [System.Collections.Generic.SortedSet[string]]::new(
                    [System.StringComparer]::Ordinal)
            }
            $null = $gap_map[$key].Add($gap.lang)
            $form_gaps.Add($gap)
        }
    }

    # Generation: only for a form/language pair whose whole surface resolves. A pair with a gap is
    # expected to fail at generation time, so generating it would only restate the gap.
    foreach ($lang in $clean_languages) {
        $switch = $generation_switches[$lang]
        $gen_dir = Join-Path $KitDir ('gen/' + $lang)
        $null = New-Item -ItemType Directory -Force -Path $gen_dir

        $gen_output = @(& $binary $form_path $switch 2>&1)
        $gen_exit = $LASTEXITCODE
        $diagnostic = ''
        foreach ($line in $gen_output) {
            $text = ([string]$line).Trim()
            if ($text -match 'Error|error:|Warning|warning:') {
                $diagnostic = $text
                break
            }
        }
        $generation_results.Add(@{
            class  = $class_name
            lang   = $lang
            ok     = ($gen_exit -eq 0)
            detail = $diagnostic
        })
    }

    $results.Add(@{
        class  = $class_name
        loaded = $true
        clean  = @($clean_languages)
        gaps   = @($form_gaps)
    })
}

# Second pass: for every gap pair, record which languages resolved it in this kit.
foreach ($entry in $kit_entries) {
    $class_name = [string]$entry['class']
    $ir_path = Join-Path $ir_dir ($class_name + '.json')
    if (-not (Test-Path -LiteralPath $ir_path)) { continue }

    $ir_document = Get-Content -LiteralPath $ir_path -Raw | ConvertFrom-Json -AsHashtable
    foreach ($form in @($ir_document['forms'])) {
        foreach ($child in @($form['children'])) {
            foreach ($role in @($child['roles_used'])) {
                $key = [string]$child['node_class'] + '|' + [string]$role
                if (-not $gap_map.ContainsKey($key)) { continue }
                if (-not $resolved_map.ContainsKey($key)) {
                    $resolved_map[$key] = [System.Collections.Generic.SortedSet[string]]::new(
                        [System.StringComparer]::Ordinal)
                }
                foreach ($lang in $languages) {
                    if (-not $gap_map[$key].Contains($lang)) {
                        $null = $resolved_map[$key].Add($lang)
                    }
                }
            }
        }
    }
}

# ----------------------------------------------------------------------------
# C4 classification for each gap (authoritative source: coverage.json)
# ----------------------------------------------------------------------------

$coverage = Get-Content -LiteralPath $coverage_path -Raw | ConvertFrom-Json -AsHashtable
$coverage_cells = @{}
$ffi_classes = [System.Collections.Generic.SortedSet[string]]::new([System.StringComparer]::Ordinal)
foreach ($row in @($coverage['rows'])) {
    $key = [string]$row['class'] + '|' + [string]$row['role']
    $coverage_cells[$key] = $row['cells']
    $null = $ffi_classes.Add([string]$row['class'])
}

function Get-GapReason {
    param([string]$Key, [string]$Lang, $Cells)

    if (-not $Cells.ContainsKey($Key)) { return 'no-ffi-path (---)' }
    $cell = $Cells[$Key][$Lang]
    if ($null -eq $cell) { return 'no-ffi-path (---)' }

    return [string]$cell['state'] + ' (' + [string]$cell['reason'] + ')'
}

# ----------------------------------------------------------------------------
# .private/kwxConformance.md
# ----------------------------------------------------------------------------

$date_text = (Get-Date).ToString('yyyy-MM-dd', [System.Globalization.CultureInfo]::InvariantCulture)

$report = [System.Text.StringBuilder]::new()
Add-Line $report '# kwx conformance sweep'
Add-Line $report ''
Add-Line $report 'Generated by `scripts/conformance.cmd`. Do not edit by hand.'
Add-Line $report ''
Add-Line $report '| field | value |'
Add-Line $report '|---|---|'
Add-Line $report ('| date | ' + $date_text + ' |')
Add-Line $report '| kit generator | `wxUiEditor --gen_conformance` (compiled generator library) |'
Add-Line $report ('| kit directory | `' + $KitDir.Replace('\', '/') + '` |')
Add-Line $report ('| forms | ' + $kit_entries.Count + ' |')
Add-Line $report ('| omitted (no parent path) | ' + $kit_omitted.Count + ' |')
Add-Line $report ('| load failures | ' + $load_failures.Count + ' |')
Add-Line $report '| sweep | `kwxgen check-ui` per form per language |'
Add-Line $report ''
Add-Line $report 'A form is `clean` for a language when `kwxgen check-ui` exits 0 for that language.'
Add-Line $report 'A gap is one `MISSING <lang> <class> <role>` line; the C4 state and reason come from'
Add-Line $report '`src/generate/ffi_ui/coverage.json`, so a gap is classified by the same source as the'
Add-Line $report 'capability matrix.'
Add-Line $report ''

if ($load_failures.Count -gt 0) {
    Add-Line $report '## Kit defects (form does not load)'
    Add-Line $report ''
    foreach ($name in $load_failures) {
        Add-Line $report ('- `' + $name + '.wxui` -- the generated project was rejected; fix the generator, not the file.')
    }
    Add-Line $report ''
}

Add-Line $report '## Sweep per language'
Add-Line $report ''
Add-Line $report '| language | forms clean | forms with gaps | missing pairs |'
Add-Line $report '|---|---|---|---|'
foreach ($lang in $languages) {
    $clean = 0
    $with_gaps = 0
    $missing_pairs = 0
    $seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($result in $results) {
        if (-not $result.loaded) { continue }
        if (@($result.clean) -contains $lang) { ++$clean }
        else { ++$with_gaps }
        foreach ($gap in @($result.gaps)) {
            if ([string]$gap.lang -ne $lang) { continue }
            $null = $seen.Add([string]$gap.class + '|' + [string]$gap.role)
        }
    }
    $missing_pairs = $seen.Count
    Add-Line $report ('| ' + $lang + ' | ' + $clean + ' | ' + $with_gaps + ' | ' + $missing_pairs + ' |')
}
Add-Line $report ''

Add-Line $report '## Generation (resolvable form/language pairs)'
Add-Line $report ''
Add-Line $report 'A form/language pair is generated only when every `(class, role)` the form uses resolves. A pair'
Add-Line $report 'with a gap is expected to fail at generation time, so generating it would only restate the gap.'
Add-Line $report ''
$gen_ok = 0
$gen_fail = 0
foreach ($result in $generation_results) {
    if ($result.ok) { ++$gen_ok } else { ++$gen_fail }
}
Add-Line $report ('Generated cleanly: ' + $gen_ok + '; failed: ' + $gen_fail + '.')
Add-Line $report ''
if ($gen_fail -gt 0) {
    Add-Line $report '| class | language | first diagnostic |'
    Add-Line $report '|---|---|---|'
    foreach ($result in $generation_results) {
        if ($result.ok) { continue }
        $detail = [string]$result.detail
        if ([string]::IsNullOrWhiteSpace($detail)) { $detail = 'generation exited non-zero' }
        $detail = $detail.Replace('|', '\\|')
        Add-Line $report ('| ' + $result.class + ' | ' + $result.lang + ' | `' + $detail + '` |')
    }
    Add-Line $report ''
}

Add-Line $report '## Gaps by class'
Add-Line $report ''
Add-Line $report 'Every `(class, role)` the kit exercises that at least one language cannot resolve.'
Add-Line $report ''
Add-Line $report '| class | role | resolves in | missing in | first C4 state |'
Add-Line $report '|---|---|---|---|---|'
$gap_keys = [System.Collections.Generic.SortedSet[string]]::new([System.StringComparer]::Ordinal)
foreach ($key in $gap_map.Keys) { $null = $gap_keys.Add($key) }
foreach ($key in $gap_keys) {
    $split = $key.IndexOf('|')
    $class_name = $key.Substring(0, $split)
    $role = $key.Substring($split + 1)
    $missing_languages = @($gap_map[$key])
    $resolved_languages = @()
    if ($resolved_map.ContainsKey($key)) { $resolved_languages = @($resolved_map[$key]) }

    $resolved_text = '_none_'
    if ($resolved_languages.Count -gt 0) { $resolved_text = ($resolved_languages -join ', ') }

    $first_reason = Get-GapReason -Key $key -Lang $missing_languages[0] -Cells $coverage_cells
    Add-Line $report ('| ' + $class_name + ' | ' + $role + ' | ' + $resolved_text + ' | ' + ($missing_languages -join ', ') + ' | `' + $first_reason + '` |')
}
Add-Line $report ''

# Generation output is scratch: the kit is exactly the .wxui files plus kit.json.
$gen_root = Join-Path $KitDir 'gen'
if (Test-Path -LiteralPath $gen_root) {
    Remove-Item -LiteralPath $gen_root -Recurse -Force
}
Get-ChildItem -LiteralPath $KitDir -Filter '*.log' -File -ErrorAction SilentlyContinue |
    Remove-Item -Force

Add-Line $report '## Class-sized batch results'
Add-Line $report ''
Add-Line $report 'A batch is one class across all six languages. `clean` lists the languages that resolve the'
Add-Line $report 'whole batch; `gap` names the pairs that do not, with the C4 state that explains it.'
Add-Line $report ''
Add-Line $report '| class | batch | clean languages | gaps | decision |'
Add-Line $report '|---|---|---|---|---|'
$batch_classes = [System.Collections.Generic.SortedSet[string]]::new([System.StringComparer]::Ordinal)
foreach ($result in $results) {
    if (-not $result.loaded) { continue }
    if ($ffi_classes.Contains([string]$result.class)) { $null = $batch_classes.Add([string]$result.class) }
}
foreach ($class_name in $batch_classes) {
    $clean = [System.Collections.Generic.SortedSet[string]]::new([System.StringComparer]::Ordinal)
    $gap_parts = [System.Collections.Generic.List[string]]::new()
    $seen_gaps = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    $all_absent = $true
    foreach ($result in $results) {
        if ([string]$result.class -ne $class_name) { continue }
        foreach ($lang in @($result.clean)) { $null = $clean.Add([string]$lang) }
        foreach ($gap in @($result.gaps)) {
            $key = [string]$gap.class + '|' + [string]$gap.role
            $gap_key = [string]$gap.lang + '|' + $key
            if (-not $seen_gaps.Add($gap_key)) { continue }
            $reason = Get-GapReason -Key $key -Lang ([string]$gap.lang) -Cells $coverage_cells
            if (-not $reason.StartsWith('absent')) { $all_absent = $false }
            $gap_parts.Add([string]$gap.lang + ' ' + [string]$gap.role + ' [' + $reason + ']')
        }
    }
    $clean_text = '_none_'
    if ($clean.Count -gt 0) { $clean_text = ($clean -join ', ') }
    $gap_text = '_none_'
    if ($gap_parts.Count -gt 0) { $gap_text = ($gap_parts -join '; ') }

    $decision = 'pass -- no gap'
    if ($gap_parts.Count -gt 0) {
        $decision = 'runtime absence -- file in .private/kwxFfiIssues.md, no emitter change'
        if (-not $all_absent) { $decision = 'descriptor/emitter gap -- fix in the batch' }
    }
    Add-Line $report ('| ' + $class_name + ' | 6 | ' + $clean_text + ' | ' + $gap_text + ' | ' + $decision + ' |')
}
Add-Line $report ''

Add-Line $report '## Scale-out backlog (no language resolves any role)'
Add-Line $report ''
Add-Line $report 'Classes the kit instantiates that no language can express yet. They are the kwxFFI wrapper'
Add-Line $report 'backlog, not a wxUiEditor defect: only `absent` and `descriptor-missing` states appear here.'
Add-Line $report ''
$no_path = [System.Collections.Generic.List[string]]::new()
foreach ($result in $results) {
    if (-not $result.loaded) { continue }
    if (@($result.clean).Count -eq 0) { $no_path.Add([string]$result.class) }
}
if ($no_path.Count -eq 0) {
    Add-Line $report '_none_'
}
else {
    Add-Line $report ('`' + ($no_path -join '`, `') + '`')
}
Add-Line $report ''

Add-Line $report '## Omitted classes (no parent chain in the generator library)'
Add-Line $report ''
if ($kit_omitted.Count -eq 0) {
    Add-Line $report '_none_'
}
else {
    $omitted_names = [System.Collections.Generic.List[string]]::new()
    foreach ($entry in $kit_omitted) { $omitted_names.Add([string]$entry['class']) }
    $omitted_names.Sort([System.StringComparer]::Ordinal)
    Add-Line $report ('`' + ($omitted_names -join '`, `') + '`')
}
Add-Line $report ''

Write-Utf8File $report_path $report.ToString()

# ----------------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------------

Write-Output ('conformance: ' + $kit_entries.Count + ' form(s), ' + $kit_omitted.Count + ' omitted, ' + $load_failures.Count + ' load failure(s)')
foreach ($lang in $languages) {
    $clean = 0
    $with_gaps = 0
    $seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($result in $results) {
        if (-not $result.loaded) { continue }
        if (@($result.clean) -contains $lang) { ++$clean }
        else { ++$with_gaps }
        foreach ($gap in @($result.gaps)) {
            if ([string]$gap.lang -ne $lang) { continue }
            $null = $seen.Add([string]$gap.class + '|' + [string]$gap.role)
        }
    }
    Write-Output ('  ' + $lang + ': clean=' + $clean + ' with-gaps=' + $with_gaps + ' missing-pairs=' + $seen.Count)
}
Write-Output ('Gap list: ' + $gap_map.Count + ' (class, role) pair(s)')
$gen_ok = 0
$gen_fail = 0
foreach ($result in $generation_results) {
    if ($result.ok) { ++$gen_ok } else { ++$gen_fail }
}
Write-Output ('Generation: ' + $gen_ok + ' clean, ' + $gen_fail + ' failed')
Write-Output ('Wrote ' + $report_path)

if ($load_failures.Count -gt 0) {
    exit 2
}
exit 0
