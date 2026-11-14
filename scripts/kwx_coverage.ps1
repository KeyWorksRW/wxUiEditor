# ============================================================================
# kwx_coverage.ps1 -- C4 coverage derivation and C5 ratchet set.
#
# Implements the plan's shared contracts C4 (coverage states) and C5 (ratchet
# set) as a repeatable command.  Every state is derived from checked-in data;
# no state is hand-set.
#
# Inputs
#   * `kwxgen coverage --inventory/--json` over the sibling kwxFFI tree.
#     This is the wrapper-presence oracle.  A `yes` means the backend wraps the
#     class -- never that the pair has ever built or run.
#   * src/generate/ffi_ui/<lang>.json -- the descriptors.  Each class's
#     `ctor.symbol` and every role `symbol` is the string grepped in the ports.
#   * src/generate/ffi_ui/verified.json -- the only source of `verified`.
#   * <repo>/.private/samples_status.md -- one `- [x| |?|-] <sample> -- <src>`
#     line per sample.  The port grepped for a sample is `<repo>/samples/<name>/`;
#     the recorded source path is the upstream C++ original, not the port.
#   * quick.wxui -- the form nodes, for the violation list.
#
# Outputs (the only files written by this script)
#   * .private/kwxSupported.md      (written by `wxUiEditor --gen_supported` when the built
#                                    binary is newer than this script, otherwise by the Phase 3
#                                    table below -- one implementation of the table per run)
#   * .private/kwxBacklog.md
#   * .private/kwxLedger.md          (the marked Phase 3 block is replaced)
#   * src/generate/ffi_ui/coverage.json
#   * .private/kwx_coverage_data/    (raw kwxgen inventory, for inspection)
#
# C4 precedence, highest first:
#   verified              verified.json entry with `state: "verified"`
#   absent                inventory says `no` for the class, or the descriptor
#                         marks the callable unsupported (unsupported[], an
#                         `idiom` of `unsupported`, or an empty symbol)
#   descriptor-missing    the class is wrapped but the descriptor has no callable
#   exercised-unverified  the descriptor symbol appears in a port's source
#   unknown               nothing observed
#
# Usage (from the repository root):
#     scripts\kwx_coverage.cmd
#
# Optional parameters (both default to the sibling repositories):
#     -KwxFfiRoot <dir>     kwxFFI checkout providing include/ and src/kwx_defs.cpp
#     -QuickProject <file>  the .wxui whose nodes are reported as violations
# ============================================================================

param(
    [string]$KwxFfiRoot = '',
    [string]$QuickProject = ''
)

$ErrorActionPreference = 'Stop'

$repo_root = Split-Path -Parent $PSScriptRoot
$sibling_root = Split-Path -Parent $repo_root

if ([string]::IsNullOrWhiteSpace($KwxFfiRoot)) {
    $KwxFfiRoot = Join-Path $sibling_root 'kwxFFI'
}
if ([string]::IsNullOrWhiteSpace($QuickProject)) {
    $QuickProject = Join-Path $sibling_root 'quick/quick.wxui'
}

$descriptor_dir = Join-Path $repo_root 'src/generate/ffi_ui'
$private_dir = Join-Path $repo_root '.private'
$data_dir = Join-Path $private_dir 'kwx_coverage_data'

$languages = @('fortran', 'go', 'julia', 'lua', 'rust', 'typescript')
$language_repos = [ordered]@{
    fortran    = 'kwxFortran'
    go         = 'kwxGO'
    julia      = 'kwxJulia'
    lua        = 'kwxLuaJIT'
    rust       = 'kwxRust'
    typescript = 'kwxTypeScript'
}
$source_extensions = @{
    fortran    = @('.f90', '.f95', '.f03', '.f08')
    go         = @('.go')
    julia      = @('.jl')
    lua        = @('.lua')
    rust       = @('.rs')
    typescript = @('.ts', '.kwxts')
}
$states = @('verified', 'absent', 'descriptor-missing', 'exercised-unverified', 'unknown')

# Auxiliary descriptor blocks map onto the wx class that owns their callables.
$aux_blocks = @(
    [ordered]@{ class = 'wxBitmap'; block = 'bitmap' },
    [ordered]@{ class = 'wxFont'; block = 'font' },
    [ordered]@{ class = 'wxStdDialogButtonSizer'; block = 'std_buttons' }
)

$ports_info = [ordered]@{}
$port_text_cache = @{}
$symbol_port_cache = @{}

# ----------------------------------------------------------------------------
# Small helpers
# ----------------------------------------------------------------------------

function Write-Utf8File {
    param([string]$Path, [string]$Text)
    [System.IO.File]::WriteAllText($Path, $Text, [System.Text.UTF8Encoding]::new($false))
}

function Read-JsonFile {
    param([string]$Path)
    return (Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json -AsHashtable)
}

function Add-Line {
    param([System.Text.StringBuilder]$Builder, [string]$Text)
    $null = $Builder.Append($Text).Append("`n")
}

function Get-SortedStrings {
    param([string[]]$Values)
    $set = [System.Collections.Generic.SortedSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($value in $Values) {
        if ($null -ne $value) {
            $null = $set.Add([string]$value)
        }
    }
    return @($set)
}

function Get-RepoPath {
    param([string]$Lang)
    return (Join-Path $sibling_root $language_repos[$Lang])
}

function Get-CallableSymbol {
    param($Callable)
    if ($null -eq $Callable) { return '' }
    if (-not $Callable.ContainsKey('symbol')) { return '' }
    return [string]$Callable['symbol']
}

function Get-CallableIdiom {
    param($Callable)
    if ($null -eq $Callable) { return '' }
    if (-not $Callable.ContainsKey('idiom')) { return '' }
    return [string]$Callable['idiom']
}

function Get-CallableReason {
    param($Callable)
    if ($null -eq $Callable) { return '' }
    if (-not $Callable.ContainsKey('reason')) { return '' }
    return [string]$Callable['reason']
}

# ----------------------------------------------------------------------------
# Descriptor model: class -> role -> callable
# ----------------------------------------------------------------------------

function Get-DescriptorCallables {
    param([hashtable]$Descriptor)

    $result = [ordered]@{}
    $classes = $Descriptor['classes']
    if ($null -eq $classes) {
        throw 'Descriptor has no classes block'
    }

    foreach ($class_name in (Get-SortedStrings @($classes.Keys))) {
        $class_block = $classes[$class_name]
        if ($null -eq $class_block) { continue }
        if (-not $result.Contains($class_name)) {
            $result[$class_name] = [ordered]@{}
        }

        if ($class_block.ContainsKey('ctor') -and $null -ne $class_block['ctor']) {
            $result[$class_name]['ctor'] = $class_block['ctor']
        }
        if ($class_block.ContainsKey('roles') -and $null -ne $class_block['roles']) {
            foreach ($role in (Get-SortedStrings @($class_block['roles'].Keys))) {
                $result[$class_name][$role] = $class_block['roles'][$role]
            }
        }
    }

    foreach ($aux in $aux_blocks) {
        if (-not $Descriptor.ContainsKey($aux.block)) { continue }
        $block = $Descriptor[$aux.block]
        if ($null -eq $block) { continue }
        if (-not $result.Contains($aux.class)) {
            $result[$aux.class] = [ordered]@{}
        }
        foreach ($role in (Get-SortedStrings @($block.Keys))) {
            $callable = $block[$role]
            # e.g. events.handler_type is a plain string, not a callable.
            if ($callable -isnot [hashtable]) { continue }
            if (-not $result[$aux.class].Contains($role)) {
                $result[$aux.class][$role] = $callable
            }
        }
    }

    return $result
}

function Get-UnsupportedMap {
    param([hashtable]$Descriptor)

    $map = @{}
    if (-not $Descriptor.ContainsKey('unsupported')) { return $map }
    $entries = $Descriptor['unsupported']
    if ($null -eq $entries) { return $map }
    foreach ($entry in @($entries)) {
        if ($null -eq $entry) { continue }
        $key = [string]$entry['class'] + '|' + [string]$entry['role']
        $map[$key] = [string]$entry['reason']
    }
    return $map
}

# ----------------------------------------------------------------------------
# Ports: samples_status.md markers and source trees
# ----------------------------------------------------------------------------

function Get-PortInfo {
    param([string]$Lang)

    $repo = Get-RepoPath $Lang
    $status_path = Join-Path $repo '.private/samples_status.md'
    $ports = [System.Collections.Generic.List[hashtable]]::new()

    if (-not (Test-Path -LiteralPath $status_path)) {
        return [ordered]@{ available = $false; status_path = $status_path; ports = @() }
    }

    foreach ($line in [System.IO.File]::ReadAllLines($status_path)) {
        $match = [regex]::Match($line, '^- \[(?<marker>.)\] (?<name>\S+) -- (?<source>.*)$')
        if (-not $match.Success) { continue }
        $name = $match.Groups['name'].Value
        $port_dir = Join-Path (Join-Path $repo 'samples') $name
        $ports.Add(@{
            name   = $name
            marker = $match.Groups['marker'].Value
            source = $match.Groups['source'].Value.Trim()
            dir    = $port_dir
            exists = (Test-Path -LiteralPath $port_dir)
        })
    }

    return [ordered]@{ available = $true; status_path = $status_path; ports = $ports.ToArray() }
}

function Get-PortTexts {
    param([string]$Lang, $PortInfo, [hashtable]$TextCache)

    if ($TextCache.ContainsKey($Lang)) {
        return $TextCache[$Lang]
    }

    $texts = [System.Collections.Generic.List[hashtable]]::new()
    if ($PortInfo.available) {
        foreach ($port in $PortInfo.ports) {
            if (-not $port.exists) { continue }
            if ($port.marker -eq '-') { continue }
            $builder = [System.Text.StringBuilder]::new()
            $files = @(Get-ChildItem -LiteralPath $port.dir -Recurse -File -ErrorAction SilentlyContinue |
                Where-Object { $source_extensions[$Lang] -contains $_.Extension.ToLowerInvariant() } |
                Sort-Object -Property FullName)
            foreach ($file in $files) {
                $null = $builder.AppendLine([System.IO.File]::ReadAllText($file.FullName))
            }
            if ($builder.Length -eq 0) { continue }
            $texts.Add(@{ name = $port.name; marker = $port.marker; text = $builder.ToString() })
        }
    }

    $TextCache[$Lang] = $texts.ToArray()
    return $TextCache[$Lang]
}

function Get-SymbolPorts {
    param([string]$Lang, [string]$Symbol, $PortInfo, [hashtable]$TextCache, [hashtable]$SymbolCache)

    if ([string]::IsNullOrEmpty($Symbol)) { return @() }
    $cache_key = $Lang + '|' + $Symbol
    if ($SymbolCache.ContainsKey($cache_key)) {
        return $SymbolCache[$cache_key]
    }

    $pattern = '(?<![A-Za-z0-9_])' + [regex]::Escape($Symbol) + '(?![A-Za-z0-9_])'
    $hits = [System.Collections.Generic.List[string]]::new()
    foreach ($port in (Get-PortTexts -Lang $Lang -PortInfo $PortInfo -TextCache $TextCache)) {
        if ([regex]::IsMatch($port.text, $pattern)) {
            $hits.Add($port.name)
        }
    }

    $SymbolCache[$cache_key] = (Get-SortedStrings $hits.ToArray())
    return $SymbolCache[$cache_key]
}

# ----------------------------------------------------------------------------
# quick.wxui: node requirements (mirrors the C2 roles_used derivation)
# ----------------------------------------------------------------------------

function Get-NodeRequirements {
    param([string]$Class, [hashtable]$Attributes, [string]$ParentClass)

    $reqs = [System.Collections.Generic.List[hashtable]]::new()
    $reqs.Add(@{ class = $Class; role = 'ctor' })

    if ($Attributes.ContainsKey('title')) { $reqs.Add(@{ class = $Class; role = 'set_title' }) }
    if ($Attributes.ContainsKey('label')) { $reqs.Add(@{ class = $Class; role = 'set_label' }) }
    if ($Attributes.ContainsKey('value')) { $reqs.Add(@{ class = $Class; role = 'set_value' }) }
    if ($Attributes.ContainsKey('size')) { $reqs.Add(@{ class = $Class; role = 'set_size' }) }
    if ($Attributes.ContainsKey('minimum_size')) { $reqs.Add(@{ class = $Class; role = 'set_min_size' }) }
    if ($Attributes.ContainsKey('center')) { $reqs.Add(@{ class = $Class; role = 'center' }) }

    if ($Attributes.ContainsKey('font')) {
        $reqs.Add(@{ class = $Class; role = 'set_font' })
        $reqs.Add(@{ class = 'wxFont'; role = 'create' })
        $reqs.Add(@{ class = 'wxFont'; role = 'delete' })
    }

    if ($Attributes.ContainsKey('bitmap')) {
        $reqs.Add(@{ class = 'wxBitmap'; role = 'from_base64' })
        $reqs.Add(@{ class = 'wxBitmap'; role = 'get_bitmap' })
        $reqs.Add(@{ class = $Class; role = 'set_bitmap' })
        $reqs.Add(@{ class = 'wxBitmap'; role = 'delete' })
    }

    # Item flags only apply when the parent is a sizer.
    if ($ParentClass -like '*Sizer') {
        if ($Attributes.ContainsKey('proportion')) { $reqs.Add(@{ class = 'wxSizerFlags'; role = 'proportion' }) }
        if ($Attributes.ContainsKey('border')) { $reqs.Add(@{ class = 'wxSizerFlags'; role = 'border' }) }
        if ($Attributes.ContainsKey('flags') -and ([string]$Attributes['flags']).Contains('wxEXPAND')) {
            $reqs.Add(@{ class = 'wxSizerFlags'; role = 'expand' })
        }
    }

    if ($Class -eq 'wxStdDialogButtonSizer') {
        $reqs.Add(@{ class = $Class; role = 'create' })
        $reqs.Add(@{ class = $Class; role = 'add_button' })
        $reqs.Add(@{ class = $Class; role = 'realize' })
    }

    foreach ($key in $Attributes.Keys) {
        if ($key.StartsWith('wxEVT_')) {
            $reqs.Add(@{ class = $Class; role = 'bind' })
            break
        }
    }

    # Return the List itself; a bare `return $reqs` would be unrolled into a
    # fixed-size Object[] by the pipeline.
    return ,$reqs
}

function Get-FormNodes {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Missing quick.wxui: $Path"
    }

    $document = [System.Xml.XmlDocument]::new()
    $document.Load($Path)
    $root = $document.SelectSingleNode('/wxUiEditorData')
    if ($null -eq $root) {
        throw "Not a wxUiEditor project: $Path"
    }

    $nodes = [System.Collections.Generic.List[hashtable]]::new()

    $project_node = $null
    foreach ($child in $root.ChildNodes) {
        if ($child.NodeType -eq 'Element' -and $child.Name -eq 'node' -and $child.GetAttribute('class') -eq 'Project') {
            $project_node = $child
            break
        }
    }
    if ($null -eq $project_node) {
        throw "quick.wxui has no Project node: $Path"
    }

    $walk = $null
    $walk = {
        param($parent_node, [int]$parent_index, [string]$parent_class)

        foreach ($child in $parent_node.ChildNodes) {
            if ($child.NodeType -ne 'Element' -or $child.Name -ne 'node') { continue }

            $attributes = @{}
            foreach ($attribute in $child.Attributes) {
                $attributes[$attribute.Name] = $attribute.Value
            }

            $class_name = [string]$attributes['class']
            $display_name = $class_name
            if ($attributes.ContainsKey('class_name')) { $display_name = [string]$attributes['class_name'] }
            elseif ($attributes.ContainsKey('var_name')) { $display_name = [string]$attributes['var_name'] }

            $index = $nodes.Count
            $nodes.Add(@{
                class         = $class_name
                name          = $display_name
                parent_class  = $parent_class
                parent_index  = $parent_index
                attributes    = $attributes
                requirements  = @()
            })

            & $walk $child $index $class_name
        }
    }
    & $walk $project_node -1 ''

    # Own-property requirements first; parent-side requirements are appended to
    # the parent below so a sizer's Add()/SetSizer() is attributed to the sizer
    # and to its window, not to the child control.
    for ($i = 0; $i -lt $nodes.Count; ++$i) {
        $node = $nodes[$i]
        $nodes[$i].requirements = Get-NodeRequirements -Class $node.class -Attributes $node.attributes -ParentClass $node.parent_class
    }

    for ($i = 0; $i -lt $nodes.Count; ++$i) {
        $node = $nodes[$i]
        if ($node.parent_index -lt 0) { continue }
        $parent = $nodes[$node.parent_index]
        if ($parent.class -like '*Sizer') {
            $role = 'add_window'
            if ($node.class -like '*Sizer') { $role = 'add_sizer' }
            $parent.requirements.Add(@{ class = $parent.class; role = $role })
        }
        elseif ($node.class -like '*Sizer') {
            $parent.requirements.Add(@{ class = $parent.class; role = 'set_sizer' })
        }
    }

    for ($i = 0; $i -lt $nodes.Count; ++$i) {
        $seen = @{}
        $unique = [System.Collections.Generic.List[hashtable]]::new()
        foreach ($requirement in $nodes[$i].requirements) {
            $key = $requirement.class + '|' + $requirement.role
            if ($seen.ContainsKey($key)) { continue }
            $seen[$key] = $true
            $unique.Add($requirement)
        }
        $nodes[$i].requirements = $unique.ToArray()
    }

    return $nodes.ToArray()
}

# ----------------------------------------------------------------------------
# Load inputs
# ----------------------------------------------------------------------------

$kwxgen_cmd = Get-Command kwxgen -ErrorAction SilentlyContinue
$kwxgen = ''
if ($null -ne $kwxgen_cmd) {
    $kwxgen = $kwxgen_cmd.Source
}
else {
    $candidate = Join-Path $KwxFfiRoot 'bin/Release/kwxgen.exe'
    if (-not (Test-Path -LiteralPath $candidate)) {
        throw "kwxgen is not on PATH and $candidate does not exist"
    }
    $kwxgen = $candidate
}

$descriptors = [ordered]@{}
foreach ($lang in $languages) {
    $path = Join-Path $descriptor_dir ($lang + '.json')
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing descriptor: $path (run scripts\gen_ffi_ui.cmd)"
    }
    $descriptor = Read-JsonFile $path
    if ($descriptor['schema'] -ne 1) {
        throw "Descriptor $path is not schema 1"
    }
    $descriptors[$lang] = $descriptor
}

$manifest_path = Join-Path $descriptor_dir 'manifest.json'
if (-not (Test-Path -LiteralPath $manifest_path)) {
    throw "Missing manifest: $manifest_path (run scripts\gen_ffi_ui.cmd)"
}
$manifest = Read-JsonFile $manifest_path
$kwxgen_version = ''
$headers_hash = ''
foreach ($lang in $languages) {
    $generated_by = $descriptors[$lang]['generated_by']
    $lang_manifest = $manifest['descriptors'][$lang]
    if ($null -eq $lang_manifest) {
        throw "manifest.json has no entry for $lang"
    }
    $version = [string]$generated_by['version']
    $hash = [string]$generated_by['headers_hash']
    if ($version -ne [string]$lang_manifest['version'] -or $hash -ne [string]$lang_manifest['headers_hash']) {
        throw "Descriptor $lang.json does not match manifest.json ($version/$hash vs $($lang_manifest['version'])/$($lang_manifest['headers_hash']))"
    }
    if ($kwxgen_version -eq '') { $kwxgen_version = $version }
    if ($headers_hash -eq '') { $headers_hash = $hash }
    if ($version -ne $kwxgen_version -or $hash -ne $headers_hash) {
        throw "Descriptors disagree on kwxgen version or headers hash"
    }
}

$verified_path = Join-Path $descriptor_dir 'verified.json'
if (-not (Test-Path -LiteralPath $verified_path)) {
    throw "Missing $verified_path (expected { ""schema"": 1, ""entries"": [] })"
}
$verified_document = Read-JsonFile $verified_path
if ($verified_document['schema'] -ne 1) {
    throw "$verified_path is not schema 1"
}
$verified_entries = @()
if ($null -ne $verified_document['entries']) {
    $verified_entries = @($verified_document['entries'])
}
$verified_roles = @{}
foreach ($entry in $verified_entries) {
    if ($null -eq $entry) { continue }
    if ([string]$entry['state'] -ne 'verified') { continue }
    foreach ($role in @($entry['roles'])) {
        $key = [string]$entry['lang'] + '|' + [string]$entry['class'] + '|' + [string]$role
        $verified_roles[$key] = [string]$entry['evidence']
    }
}

$null = New-Item -ItemType Directory -Force -Path $data_dir
$inventory_md = Join-Path $data_dir 'inventory.md'
$inventory_json = Join-Path $data_dir 'inventory.json'

& $kwxgen coverage --headers (Join-Path $KwxFfiRoot 'include') --defs (Join-Path $KwxFfiRoot 'src/kwx_defs.cpp') --inventory $inventory_md --json $inventory_json | Out-Null
if ($LASTEXITCODE -ne 0) {
    throw "kwxgen coverage failed with exit code $LASTEXITCODE"
}
if (-not (Test-Path -LiteralPath $inventory_json)) {
    throw "kwxgen coverage did not write $inventory_json"
}

$inventory = Read-JsonFile $inventory_json
$inventory_classes = 0
foreach ($line in [System.IO.File]::ReadAllLines($inventory_md)) {
    if (-not $line.StartsWith('| ')) { continue }
    if ($line.StartsWith('| class ')) { continue }
    if ($line.StartsWith('|---')) { continue }
    ++$inventory_classes
}

$wrapped = @{}
foreach ($lang in $languages) {
    $set = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($name in @($inventory[$lang]['wrapped_classes'])) {
        if ($null -ne $name) { $null = $set.Add([string]$name) }
    }
    foreach ($name in @($inventory[$lang]['consumer_provided_classes'])) {
        if ($null -ne $name) { $null = $set.Add([string]$name) }
    }
    $wrapped[$lang] = $set
}

foreach ($lang in $languages) {
    $ports_info[$lang] = Get-PortInfo $lang
}

$port_status = [ordered]@{}
$missing_status = [System.Collections.Generic.List[string]]::new()
foreach ($lang in $languages) {
    $info = $ports_info[$lang]
    $greppable = 0
    if ($info.available) {
        foreach ($port in $info.ports) {
            if ($port.exists -and $port.marker -ne '-') { ++$greppable }
        }
    }
    else {
        $missing_status.Add($lang)
    }
    $port_status[$lang] = [ordered]@{
        path      = $info.status_path
        available = [bool]$info.available
        entries   = @($info.ports).Count
        ports     = $greppable
    }
}


$callables = [ordered]@{}
$unsupported = @{}
foreach ($lang in $languages) {
    $callables[$lang] = Get-DescriptorCallables $descriptors[$lang]
    $unsupported[$lang] = Get-UnsupportedMap $descriptors[$lang]
}

# ----------------------------------------------------------------------------
# Build the (class, role) row universe and resolve one C4 state per cell
# ----------------------------------------------------------------------------

$row_key_set = [System.Collections.Generic.SortedSet[string]]::new([System.StringComparer]::Ordinal)
foreach ($lang in $languages) {
    foreach ($class_name in $callables[$lang].Keys) {
        foreach ($role in $callables[$lang][$class_name].Keys) {
            $null = $row_key_set.Add($class_name + '|' + $role)
        }
    }
    foreach ($key in $unsupported[$lang].Keys) {
        $null = $row_key_set.Add($key)
    }
}

$rows = [System.Collections.Generic.List[hashtable]]::new()
$row_index = @{}

foreach ($row_key in $row_key_set) {
    $split = $row_key.IndexOf('|')
    $class_name = $row_key.Substring(0, $split)
    $role = $row_key.Substring($split + 1)

    $cells = [ordered]@{}
    $verified_count = 0

    foreach ($lang in $languages) {
        $state = 'unknown'
        $symbol = ''
        $reason = ''
        $ports = @()

        $callable = $null
        if ($callables[$lang].Contains($class_name) -and $callables[$lang][$class_name].Contains($role)) {
            $callable = $callables[$lang][$class_name][$role]
        }
        $unsupported_reason = ''
        if ($unsupported[$lang].ContainsKey($row_key)) {
            $unsupported_reason = $unsupported[$lang][$row_key]
        }

        if ($verified_roles.ContainsKey($lang + '|' + $row_key)) {
            $state = 'verified'
            $reason = 'verified.json'
            $symbol = Get-CallableSymbol $callable
        }
        elseif (-not $wrapped[$lang].Contains($class_name)) {
            $state = 'absent'
            $reason = 'inventory:no'
            $symbol = Get-CallableSymbol $callable
        }
        elseif ($unsupported_reason -ne '') {
            $state = 'absent'
            $reason = 'unsupported:' + $unsupported_reason
            $symbol = Get-CallableSymbol $callable
        }
        elseif ($null -ne $callable) {
            $symbol = Get-CallableSymbol $callable
            $idiom = Get-CallableIdiom $callable
            $callable_reason = Get-CallableReason $callable
            if ($idiom -eq 'unsupported' -or [string]::IsNullOrEmpty($symbol)) {
                $state = 'absent'
                if ($callable_reason -ne '') {
                    $reason = 'unsupported:' + $callable_reason
                }
                else {
                    $reason = 'descriptor:empty-symbol'
                }
            }
            else {
                $ports = @(Get-SymbolPorts -Lang $lang -Symbol $symbol -PortInfo $ports_info[$lang] -TextCache $port_text_cache -SymbolCache $symbol_port_cache)
                if ($ports.Count -gt 0) {
                    $state = 'exercised-unverified'
                    $reason = 'port-grep'
                }
            }
        }
        else {
            $state = 'descriptor-missing'
            $reason = 'descriptor:none'
        }

        if ($state -eq 'verified') { ++$verified_count }

        $cells[$lang] = [ordered]@{
            state  = $state
            symbol = $symbol
            reason = $reason
            ports  = @($ports)
        }
    }

    $row = @{
        class          = $class_name
        role           = $role
        key            = $row_key
        cells          = $cells
        verified_count = $verified_count
        ratchet        = ($verified_count -eq $languages.Count)
    }
    $rows.Add($row)
    $row_index[$row_key] = $row
}

$ratchet_rows = @($rows | Where-Object { $_.ratchet })

$counts = [ordered]@{}
foreach ($lang in $languages) {
    $per_state = [ordered]@{}
    foreach ($state in $states) { $per_state[$state] = 0 }
    foreach ($row in $rows) {
        $per_state[$row.cells[$lang].state] = $per_state[$row.cells[$lang].state] + 1
    }
    $counts[$lang] = $per_state
}

$form_nodes = Get-FormNodes $QuickProject

$node_violations = [System.Collections.Generic.List[hashtable]]::new()
$node_findings = [System.Collections.Generic.List[hashtable]]::new()
$unresolved_requirements = [System.Collections.Generic.List[hashtable]]::new()

foreach ($node in $form_nodes) {
    $blocking = [System.Collections.Generic.List[string]]::new()
    foreach ($lang in $languages) {
        $all_verified = $true
        foreach ($requirement in $node.requirements) {
            $key = $requirement.class + '|' + $requirement.role
            if (-not $row_index.ContainsKey($key)) { continue }
            if ($row_index[$key].cells[$lang].state -ne 'verified') {
                $all_verified = $false
                break
            }
        }
        if (-not $all_verified) { $blocking.Add($lang) }
    }

    $role_labels = [System.Collections.Generic.List[string]]::new()
    foreach ($requirement in $node.requirements) {
        if ($requirement.class -eq $node.class) {
            $role_labels.Add($requirement.role)
        }
        else {
            $role_labels.Add($requirement.class + '.' + $requirement.role)
        }
    }

    $node_violations.Add(@{
        name          = $node.name
        class         = $node.class
        roles         = (Get-SortedStrings $role_labels.ToArray())
        blocking      = $blocking.ToArray()
        requirements  = $node.requirements
    })

    foreach ($requirement in $node.requirements) {
        $key = $requirement.class + '|' + $requirement.role
        if (-not $row_index.ContainsKey($key)) {
            $unresolved_requirements.Add(@{
                node   = $node.name
                class  = $requirement.class
                role   = $requirement.role
            })
            continue
        }
        foreach ($lang in $languages) {
            $state = $row_index[$key].cells[$lang].state
            if ($state -ne 'absent' -and $state -ne 'descriptor-missing') { continue }
            $node_findings.Add(@{
                node   = $node.name
                class  = $requirement.class
                role   = $requirement.role
                lang   = $lang
                state  = $state
                reason = $row_index[$key].cells[$lang].reason
            })
        }
    }
}

$date_text = (Get-Date).ToString('yyyy-MM-dd', [System.Globalization.CultureInfo]::InvariantCulture)

# ----------------------------------------------------------------------------
# .private/kwxSupported.md
# ----------------------------------------------------------------------------

$supported = [System.Text.StringBuilder]::new()
Add-Line $supported '# kwx supported matrix'
Add-Line $supported ''
Add-Line $supported 'Generated by `scripts/kwx_coverage.ps1` (C4 coverage derivation). Do not edit by hand.'
Add-Line $supported ''
Add-Line $supported '| field | value |'
Add-Line $supported '|---|---|'
Add-Line $supported ('| date | ' + $date_text + ' |')
Add-Line $supported ('| kwxgen | ' + $kwxgen_version + ' |')
Add-Line $supported ('| headers_hash | ' + $headers_hash + ' |')
Add-Line $supported ('| inventory classes | ' + $inventory_classes + ' |')
Add-Line $supported '| descriptors | `src/generate/ffi_ui/<lang>.json` (schema 1, kwxgen-generated) |'
Add-Line $supported ('| `verified.json` entries | ' + $verified_entries.Count + ' |')
Add-Line $supported '| states | `verified`, `absent`, `descriptor-missing`, `exercised-unverified`, `unknown` |'
Add-Line $supported ''
Add-Line $supported 'Precedence (C4): `verified` -> `absent` -> `descriptor-missing` -> `exercised-unverified` -> `unknown`.'
Add-Line $supported '`kwxgen coverage --inventory` `yes` is wrapper presence only and never yields `verified`.'
Add-Line $supported 'Ports are grepped by descriptor symbol in `<repo>/samples/<sample>/` for every port whose'
Add-Line $supported '`.private/samples_status.md` marker is `[x]`, `[?]` or `[ ]`; `[-]` is excluded.'
Add-Line $supported ''
Add-Line $supported '## Counts per language'
Add-Line $supported ''
Add-Line $supported ('| language | ' + ($states -join ' | ') + ' | total |')
Add-Line $supported ('|---|' + (('---|') * ($states.Count + 1)))
foreach ($lang in $languages) {
    $cells = @()
    $total = 0
    foreach ($state in $states) {
        $value = $counts[$lang][$state]
        $cells += [string]$value
        $total += $value
    }
    Add-Line $supported ('| ' + $lang + ' | ' + ($cells -join ' | ') + ' | ' + $total + ' |')
}
Add-Line $supported ''
Add-Line $supported '## Ratchet set (verified 6/6)'
Add-Line $supported ''
if ($ratchet_rows.Count -eq 0) {
    Add-Line $supported ('None. No `(class, role)` pair is verified in all six languages: `verified.json` has ' + $verified_entries.Count + ' entries.')
    Add-Line $supported 'Every `quick.wxui` node is therefore outside the ratchet set; see `.private/kwxBacklog.md`.'
}
else {
    foreach ($row in $ratchet_rows) {
        Add-Line $supported ('- `' + $row.class + '.' + $row.role + '`')
    }
}
Add-Line $supported ''
Add-Line $supported '## Matrix'
Add-Line $supported ''
Add-Line $supported ('| class | role | ' + ($languages -join ' | ') + ' | ratchet |')
Add-Line $supported ('|---|---|' + (('---|') * ($languages.Count + 1)))
foreach ($row in $rows) {
    $cells = @()
    foreach ($lang in $languages) {
        $cells += '`' + $row.cells[$lang].state + '`'
    }
    $ratchet_cell = 'no (' + $row.verified_count + '/6)'
    if ($row.ratchet) { $ratchet_cell = 'yes' }
    Add-Line $supported ('| ' + $row.class + ' | ' + $row.role + ' | ' + ($cells -join ' | ') + ' | ' + $ratchet_cell + ' |')
}
Add-Line $supported ''

# The .private/kwxSupported.md write is deferred until after coverage.json exists: the in-app
# writer (--gen_supported) reads that report. See the end of the coverage.json section.

# ----------------------------------------------------------------------------
# .private/kwxBacklog.md
# ----------------------------------------------------------------------------

$backlog_groups = [ordered]@{
    'Promotable: verified 5/6'                                        = @()
    'verified 4/6'                                                    = @()
    'descriptor-missing (actionable in wxUiEditor or kwxFFI)'         = @()
    'absent (actionable only in kwxFFI)'                              = @()
    'remaining (verified below 4/6, no absent or descriptor-missing)' = @()
}

foreach ($row in $rows) {
    $group = 'remaining (verified below 4/6, no absent or descriptor-missing)'
    if ($row.verified_count -eq 5) {
        $group = 'Promotable: verified 5/6'
    }
    elseif ($row.verified_count -eq 4) {
        $group = 'verified 4/6'
    }
    else {
        $has_descriptor_missing = $false
        $has_absent = $false
        foreach ($lang in $languages) {
            if ($row.cells[$lang].state -eq 'descriptor-missing') { $has_descriptor_missing = $true }
            if ($row.cells[$lang].state -eq 'absent') { $has_absent = $true }
        }
        if ($has_descriptor_missing) {
            $group = 'descriptor-missing (actionable in wxUiEditor or kwxFFI)'
        }
        elseif ($has_absent) {
            $group = 'absent (actionable only in kwxFFI)'
        }
    }
    $backlog_groups[$group] += $row
}

$backlog = [System.Text.StringBuilder]::new()
Add-Line $backlog '# kwx backlog'
Add-Line $backlog ''
Add-Line $backlog 'Generated by `scripts/kwx_coverage.ps1`. Do not edit by hand.'
Add-Line $backlog ''
Add-Line $backlog ('Date: ' + $date_text + '; kwxgen ' + $kwxgen_version + '; ratchet set: ' + $ratchet_rows.Count + ' pair(s).')
Add-Line $backlog ''
Add-Line $backlog 'Ordering (plan Phase 3): pairs verified 5/6 (naming the blocking language), then 4/6,'
Add-Line $backlog 'then `descriptor-missing` pairs, then `absent` pairs, then everything else. A pair becomes'
Add-Line $backlog 'addable to `quick.wxui` only when it is `verified` in all six languages (C5).'
Add-Line $backlog ''

foreach ($group_name in $backlog_groups.Keys) {
    $group_rows = @($backlog_groups[$group_name])
    Add-Line $backlog ('## ' + $group_name + ' (' + $group_rows.Count + ')')
    Add-Line $backlog ''
    if ($group_rows.Count -eq 0) {
        Add-Line $backlog '_none_'
        Add-Line $backlog ''
        continue
    }
    Add-Line $backlog '| class | role | verified | blocking languages | states |'
    Add-Line $backlog '|---|---|---|---|---|'
    foreach ($row in $group_rows) {
        $blocking = [System.Collections.Generic.List[string]]::new()
        $state_text = [System.Collections.Generic.List[string]]::new()
        foreach ($lang in $languages) {
            if ($row.cells[$lang].state -ne 'verified') { $blocking.Add($lang) }
            $state_text.Add($lang + '=' + $row.cells[$lang].state)
        }
        $blocking_text = '_none_'
        if ($blocking.Count -gt 0) { $blocking_text = ($blocking -join ', ') }
        Add-Line $backlog ('| ' + $row.class + ' | ' + $row.role + ' | ' + $row.verified_count + '/6 | ' + $blocking_text + ' | ' + ($state_text -join '; ') + ' |')
    }
    Add-Line $backlog ''
}

# The Phase 5 demotion record. It is written here, not derived from the current form, because
# `quick.wxui` no longer contains the demoted nodes -- the record has to survive regeneration.
# The seven rows are the Phase 3 violation list for the pre-Phase-5 form; the two form nodes are
# the documented exception (a .wxui project cannot exist without a form node). Each pair is
# unverified, not failed. Phase 14 replaces this section as pairs are promoted.
Add-Line $backlog '## Demoted in Phase 5 (2026-11-12) -- pair unverified, not failed (7)'
Add-Line $backlog ''
Add-Line $backlog 'Every row below is outside the ratchet set, so the node was removed from `quick.wxui` (C5).'
Add-Line $backlog '`verified.json` is empty, so no pair is 6/6 verified; the demoted content is exercised'
Add-Line $backlog 'through `quick/tests/quick_probe/probe.wxui` until Phase 14 re-promotes it.'
Add-Line $backlog ''
Add-Line $backlog 'demoted: wxTextCtrl ctor — blocked by fortran, go, julia, lua, rust, typescript — node textCtrl; pair unverified, not failed'
Add-Line $backlog 'demoted: wxBoxSizer ctor, set_min_size, add_window, add_sizer — blocked by fortran, go, julia, lua, rust, typescript — node dlg_sizer (subtree removed with it); pair unverified, not failed'
Add-Line $backlog 'demoted: wxTextCtrl ctor, set_value, set_font, wxFont.create, wxFont.delete, wxSizerFlags.proportion, wxSizerFlags.expand — blocked by fortran, go, julia, lua, rust, typescript — node m_text_ctrl; pair unverified, not failed'
Add-Line $backlog 'demoted: wxStaticBitmap ctor, set_bitmap, wxBitmap.from_base64, wxBitmap.get_bitmap, wxBitmap.delete — blocked by fortran, go, julia, lua, rust, typescript — node wxStaticBitmap; pair unverified, not failed'
Add-Line $backlog 'demoted: wxStdDialogButtonSizer ctor, create, add_button, realize, wxSizerFlags.expand — blocked by fortran, go, julia, lua, rust, typescript — node wxStdDialogButtonSizer; pair unverified, not failed'
Add-Line $backlog 'demoted: wxFrame ctor, set_title, set_min_size — blocked by fortran, go, julia, lua, rust, typescript — node kwxMainFrame RETAINED as the project skeleton; pair unverified, not failed'
Add-Line $backlog 'demoted: wxDialog ctor, set_title, set_size, center, set_sizer, bind — blocked by fortran, go, julia, lua, rust, typescript — node MainTest RETAINED as the project skeleton; pair unverified, not failed'
Add-Line $backlog ''
Add-Line $backlog 'Exception (user decision, governing this phase): the two form nodes and their form-level attributes'
Add-Line $backlog 'stay in `quick.wxui`. A .wxui project cannot exist without form nodes, so they are the project'
Add-Line $backlog 'skeleton rather than ratcheted content. No content node remains in the form.'
Add-Line $backlog ''

Write-Utf8File (Join-Path $private_dir 'kwxBacklog.md') $backlog.ToString()

# ----------------------------------------------------------------------------
# src/generate/ffi_ui/coverage.json
# ----------------------------------------------------------------------------

$json_rows = [System.Collections.Generic.List[object]]::new()
foreach ($row in $rows) {
    $cells = [ordered]@{}
    foreach ($lang in $languages) {
        $cells[$lang] = [ordered]@{
            state  = $row.cells[$lang].state
            symbol = $row.cells[$lang].symbol
            reason = $row.cells[$lang].reason
            ports  = @($row.cells[$lang].ports)
        }
    }
    $json_rows.Add([ordered]@{
        class          = $row.class
        role           = $row.role
        verified_count = $row.verified_count
        ratchet        = $row.ratchet
        cells          = $cells
    })
}

$json_ratchet = [System.Collections.Generic.List[object]]::new()
foreach ($row in $ratchet_rows) {
    $json_ratchet.Add([ordered]@{ class = $row.class; role = $row.role })
}

$json_counts = [ordered]@{}
foreach ($lang in $languages) {
    $json_counts[$lang] = [ordered]@{}
    foreach ($state in $states) {
        $json_counts[$lang][$state] = $counts[$lang][$state]
    }
}

$json_form_nodes = [System.Collections.Generic.List[object]]::new()
foreach ($node in $form_nodes) {
    $requirements = [System.Collections.Generic.List[object]]::new()
    foreach ($requirement in $node.requirements) {
        $requirements.Add([ordered]@{ class = $requirement.class; role = $requirement.role })
    }
    $json_form_nodes.Add([ordered]@{
        name         = $node.name
        class        = $node.class
        requirements = $requirements
    })
}

$json_violations = [System.Collections.Generic.List[object]]::new()
foreach ($violation in $node_violations) {
    $json_violations.Add([ordered]@{
        name             = $violation.name
        class            = $violation.class
        roles            = @($violation.roles)
        blocking_langs   = @($violation.blocking)
    })
}

$coverage = [ordered]@{
    schema      = 1
    generated   = $date_text
    kwxgen      = [ordered]@{
        version      = $kwxgen_version
        headers_hash = $headers_hash
        report       = 'kwxgen coverage --headers include --defs src/kwx_defs.cpp'
        class_count  = $inventory_classes
    }
    port_status = $port_status
    languages   = @($languages)
    states      = @($states)
    verified_entries = $verified_entries.Count
    counts      = $json_counts
    ratchet     = $json_ratchet
    rows        = $json_rows
    form        = [ordered]@{
        project           = (Split-Path -Leaf $QuickProject)
        nodes             = $json_form_nodes
        violations        = $json_violations
        unresolved        = @($unresolved_requirements | ForEach-Object { [ordered]@{ node = $_.node; class = $_.class; role = $_.role } })
        unresolved_count  = $unresolved_requirements.Count
    }
}

$coverage_text = ($coverage | ConvertTo-Json -Depth 20) + "`n"
Write-Utf8File (Join-Path $descriptor_dir 'coverage.json') $coverage_text

# ----------------------------------------------------------------------------
# .private/kwxSupported.md -- one implementation of the table
#
# `wxUiEditor --gen_supported` is the full-surface implementation (every generator class). It is
# used whenever the built binary is newer than this script; when the binary is absent or older,
# the Phase 3 table assembled above is written instead, so the script keeps working either way.
# ----------------------------------------------------------------------------

$supported_path = Join-Path $private_dir 'kwxSupported.md'
$generated_written = $false

$binary_candidates = @(
    (Join-Path $repo_root 'bin/Release/wxUiEditor.exe'),
    (Join-Path $repo_root 'build/bin/Release/wxUiEditor.exe'),
    (Join-Path $repo_root 'bin/Debug/wxUiEditor.exe'),
    (Join-Path $repo_root 'build/bin/Debug/wxUiEditor.exe')
)
$supported_binary = $null
foreach ($candidate in $binary_candidates) {
    if (Test-Path -LiteralPath $candidate) {
        $supported_binary = $candidate
        break
    }
}

if ($null -ne $supported_binary) {
    $binary_time = (Get-Item -LiteralPath $supported_binary).LastWriteTimeUtc
    $script_time = (Get-Item -LiteralPath $PSCommandPath).LastWriteTimeUtc
    if ($binary_time -gt $script_time) {
        $proc = Start-Process -FilePath $supported_binary `
            -ArgumentList ('--gen_supported "' + $supported_path + '"') `
            -WorkingDirectory $repo_root -Wait -NoNewWindow -PassThru
        if ($proc.ExitCode -eq 0 -and (Test-Path -LiteralPath $supported_path)) {
            $generated_written = $true
        }
        else {
            Write-Warning ('--gen_supported failed (exit ' + $proc.ExitCode + '); using the script table')
        }
    }
}

if ($generated_written) {
    Write-Output ('kwxSupported.md: written by ' + $supported_binary + ' (--gen_supported)')
}
else {
    Write-Utf8File $supported_path $supported.ToString()
    Write-Output 'kwxSupported.md: written by the script (no newer wxUiEditor binary)'
}

# ----------------------------------------------------------------------------
# .private/kwxLedger.md -- replace the marked Phase 3 block
# ----------------------------------------------------------------------------

$ledger_path = Join-Path $private_dir 'kwxLedger.md'
$ledger_begin = '<!-- kwx-coverage:begin -->'
$ledger_end = '<!-- kwx-coverage:end -->'

$ledger_block = [System.Text.StringBuilder]::new()
Add-Line $ledger_block $ledger_begin
Add-Line $ledger_block ('## Phase 3 -- coverage derivation (' + $date_text + ')')
Add-Line $ledger_block ''
Add-Line $ledger_block '- Command: `scripts\kwx_coverage.cmd` (C4 states, C5 ratchet set, promotion backlog).'
Add-Line $ledger_block ('- Inputs: `kwxgen ' + $kwxgen_version + '` coverage inventory (' + $inventory_classes + ' classes, headers_hash `' + $headers_hash + '`); six descriptors (schema 1); six `<repo>/.private/samples_status.md`; `src/generate/ffi_ui/verified.json` (' + $verified_entries.Count + ' entries).')
Add-Line $ledger_block '- `samples_status.md` records the upstream C++ source path; the port tree grepped is `<repo>/samples/<sample>/`.'
if ($missing_status.Count -eq 0) {
    Add-Line $ledger_block '- Port status files: all six `samples_status.md` present and parsed.'
}
else {
    Add-Line $ledger_block ('- Port status files MISSING for: ' + ($missing_status -join ', ') + ' -- their samples are treated as `[ ]` and no port was grepped.')
}
Add-Line $ledger_block '- States: `verified` (0), `absent`, `descriptor-missing`, `exercised-unverified`, `unknown`. `kwxgen coverage --inventory` `yes` is wrapper presence only.'
$state_line = '- State counts: '
$state_parts = [System.Collections.Generic.List[string]]::new()
foreach ($lang in $languages) {
    $per_state = [System.Collections.Generic.List[string]]::new()
    foreach ($state in $states) {
        $per_state.Add($state + ' ' + $counts[$lang][$state])
    }
    $state_parts.Add($lang + ' (' + ($per_state -join ', ') + ')')
}
$state_line = $state_line + ($state_parts -join '; ')
Add-Line $ledger_block $state_line
Add-Line $ledger_block ('- Ratchet set (verified 6/6): ' + $(if ($ratchet_rows.Count -eq 0) { 'none -- `verified.json` has no entries, so no pair is 6/6 verified yet (expected before the language phases record drives).' } else { ($ratchet_rows.Count.ToString() + ' pair(s)') }))
Add-Line $ledger_block ''
Add-Line $ledger_block '### `quick.wxui` nodes not 6/6 verified (expected Phase 5 form reduction)'
Add-Line $ledger_block ''
Add-Line $ledger_block '| node | class | roles required | blocking languages |'
Add-Line $ledger_block '|---|---|---|---|'
foreach ($violation in $node_violations) {
    $blocking_text = '_none_'
    if ($violation.blocking.Count -gt 0) { $blocking_text = ($violation.blocking -join ', ') }
    Add-Line $ledger_block ('| ' + $violation.name + ' | ' + $violation.class + ' | ' + ($violation.roles -join ', ') + ' | ' + $blocking_text + ' |')
}
Add-Line $ledger_block ''
Add-Line $ledger_block '### Node-level hard findings (`absent` / `descriptor-missing`)'
Add-Line $ledger_block ''
if ($node_findings.Count -eq 0) {
    Add-Line $ledger_block '_none_'
}
else {
    Add-Line $ledger_block '| node | class | role | language | state | reason |'
    Add-Line $ledger_block '|---|---|---|---|---|---|'
    foreach ($finding in $node_findings) {
        Add-Line $ledger_block ('| ' + $finding.node + ' | ' + $finding.class + ' | ' + $finding.role + ' | ' + $finding.lang + ' | ' + $finding.state + ' | ' + $finding.reason + ' |')
    }
}
Add-Line $ledger_block ''
Add-Line $ledger_block '### Requirements with no C4 cell (C1 descriptor gap)'
Add-Line $ledger_block ''
if ($unresolved_requirements.Count -eq 0) {
    Add-Line $ledger_block '_none_'
}
else {
    Add-Line $ledger_block '| node | class | role | note |'
    Add-Line $ledger_block '|---|---|---|---|'
    foreach ($unresolved in $unresolved_requirements) {
        Add-Line $ledger_block ('| ' + $unresolved.node + ' | ' + $unresolved.class + ' | ' + $unresolved.role + ' | no descriptor entry in any language; the role cannot be expressed through C1 |')
    }
}
Add-Line $ledger_block $ledger_end

$ledger_text = ''
if (Test-Path -LiteralPath $ledger_path) {
    $ledger_text = [System.IO.File]::ReadAllText($ledger_path)
}

$block_text = $ledger_block.ToString().TrimEnd("`n")
$begin_index = $ledger_text.IndexOf($ledger_begin, [System.StringComparison]::Ordinal)
if ($begin_index -ge 0) {
    $end_index = $ledger_text.IndexOf($ledger_end, $begin_index, [System.StringComparison]::Ordinal)
    if ($end_index -lt 0) {
        throw "kwxLedger.md has $ledger_begin without $ledger_end"
    }
    $end_index += $ledger_end.Length
    $ledger_text = $ledger_text.Substring(0, $begin_index) + $block_text + $ledger_text.Substring($end_index)
}
else {
    if (-not $ledger_text.EndsWith("`n")) { $ledger_text += "`n" }
    $ledger_text += "`n" + $block_text + "`n"
}

Write-Utf8File $ledger_path $ledger_text

# ----------------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------------

Write-Output ('kwxgen: ' + $kwxgen_version + ' (headers_hash ' + $headers_hash + ')')
Write-Output ('Rows: ' + $rows.Count + ' (class, role) pairs; ratchet set: ' + $ratchet_rows.Count)
foreach ($lang in $languages) {
    $parts = [System.Collections.Generic.List[string]]::new()
    foreach ($state in $states) { $parts.Add($state + '=' + $counts[$lang][$state]) }
    Write-Output ('  ' + $lang + ': ' + ($parts -join ' '))
}
Write-Output ('quick.wxui nodes: ' + $form_nodes.Count + '; not 6/6 verified: ' + $node_violations.Count)
Write-Output ('Wrote .private/kwxSupported.md, .private/kwxBacklog.md, .private/kwxLedger.md, src/generate/ffi_ui/coverage.json')
