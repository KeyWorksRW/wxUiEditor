# ============================================================================
# Rewrites src/generate/ffi_ui/manifest.json from the six descriptors.
#
# Called by scripts/gen_ffi_ui.cmd after kwxgen has regenerated the descriptors.
# The manifest records the kwxgen version and the headers hash each descriptor
# was built from, so wxUiEditor can reject a descriptor that no longer matches
# the kwxFFI header surface.
# ============================================================================

$ErrorActionPreference = 'Stop'

$repo_root = Split-Path -Parent $PSScriptRoot
$descriptor_dir = Join-Path $repo_root 'src/generate/ffi_ui'
$languages = @('fortran', 'go', 'julia', 'lua', 'rust', 'typescript')

$descriptors = [ordered]@{}

foreach ($lang in $languages) {
    $path = Join-Path $descriptor_dir ($lang + '.json')
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing descriptor: $path"
    }

    $json = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
    $descriptors[$lang] = [ordered]@{
        version      = $json.generated_by.version
        headers_hash = $json.generated_by.headers_hash
    }
}

$manifest = [ordered]@{
    schema      = 1
    tool        = 'kwxgen'
    descriptors = $descriptors
}

$output_path = Join-Path $descriptor_dir 'manifest.json'
$text = ($manifest | ConvertTo-Json -Depth 4) + "`n"
[System.IO.File]::WriteAllText($output_path, $text, [System.Text.UTF8Encoding]::new($false))

Write-Output "Wrote $output_path"
