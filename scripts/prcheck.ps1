function prcheck {
    # Navigate to the repository root
    $scriptDir = Split-Path -Parent $PSCommandPath
    $repoRoot = Split-Path -Parent $scriptDir
    Push-Location $repoRoot

    try {
        # Run typos spell checker (config in _typos.toml)
        typos .

        $exitCode = $LASTEXITCODE

        if ($exitCode -ne 0) {
            Write-Host ""
            Write-Host "=================================== ERROR ===================================" -ForegroundColor Red
            Write-Host "Spell check failed. To fix typos automatically, run: typos --write-changes" -ForegroundColor Red
            Write-Host "" -ForegroundColor Red
            Write-Host "To suppress false positives, edit _typos.toml in the repository root:" -ForegroundColor Red
            Write-Host "  - Add words to [default.extend-words] section" -ForegroundColor Red
            Write-Host "  - Add identifiers to [default.extend-identifiers] section" -ForegroundColor Red
            Write-Host "=============================================================================" -ForegroundColor Red
        }

        return $exitCode
    }
    finally {
        Pop-Location
    }
}

# Run the function if script is executed directly
prcheck
