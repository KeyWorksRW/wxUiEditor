#!/usr/bin/env bash
# ============================================================================
# Regenerates src/generate/ffi_ui/<lang>.json for the six kwx languages.
#
# Requires kwxgen on PATH (built from the sibling kwxFFI repository). The
# descriptors are generated output -- never hand-edit them. Each run also
# rewrites manifest.json so src/generate/writers/ffi_ui.cpp can detect a
# descriptor that is stale relative to the kwxFFI headers.
#
# verified.json is different: it holds curated verified-emitter evidence and is
# intentionally untracked, so a fresh checkout does not have it. When it is
# absent this script seeds an empty schema-1 stub, which keeps coverage.json
# generation (and kwx_coverage.ps1) working with zero verified-emitter cells. An
# existing verified.json is never overwritten.
#
# This is the bash form of the retired scripts/gen_ffi_ui.cmd. kwxgen is an
# executable, so there is no cmd.exe layer and no batch file. The manifest
# writer stays in PowerShell (gen_ffi_ui_manifest.ps1): it reads JSON and
# hashes the binary, which PowerShell already does correctly.
#
# Usage (from anywhere):
#     bash .tts/gen_ffi_ui.sh
# ============================================================================

set -u

Say() { printf '%s\n' "$*"; }

pwd_win() {
    local here=''
    if here="$(pwd -W 2>/dev/null)" && [ -n "$here" ]; then
        printf '%s' "$here"
        return 0
    fi
    here="$(pwd)"
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -m "$here"
        return 0
    fi
    printf '%s' "$here"
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd_win)"
ROOT="$(cd "$SCRIPT_DIR/.." >/dev/null 2>&1 && pwd_win)"

# Resolve kwxgen. Priority: KWXGEN_EXE (explicit override), then the canonical
# source-tree Release build that .tts/build-release.ps1 produces
# (kwxFFI\bin\Release\kwxgen.exe) -- the same artifact that script mirrors to
# C:\rwCode\bin, so a hash difference between the two means the mirror has not
# run since the last build. A standalone configure of kwxFFI\kwxgen (its own
# CMake build directory) writes kwxFFI\kwxgen\bin\Release instead; build-release
# never touches that tree, so it silently goes stale and must only ever be a
# fallback. PATH order is never consulted while a source-tree build exists,
# because two builds of the same source can both answer --classes and still
# differ in behaviour (measured 2026-11-14: e3c08373... vs d1a566bc...).
KWXGEN=''
if [ -n "${KWXGEN_EXE:-}" ]; then
    KWXGEN="$KWXGEN_EXE"
fi

REPO_KWXGEN="$ROOT/../kwxFFI/bin/Release/kwxgen.exe"
if [ -z "$KWXGEN" ] && [ -f "$REPO_KWXGEN" ]; then
    KWXGEN="$REPO_KWXGEN"
fi

# Not maintained by the kwxFFI build task -- use it only when the canonical
# build is absent, and say so, because it can predate the current source.
STANDALONE_KWXGEN="$ROOT/../kwxFFI/kwxgen/bin/Release/kwxgen.exe"
if [ -z "$KWXGEN" ] && [ -f "$STANDALONE_KWXGEN" ]; then
    KWXGEN="$STANDALONE_KWXGEN"
    Say 'Note: using the standalone kwxFFI/kwxgen build (not the canonical kwxFFI/bin/Release).'
    Say '      Run the kwxFFI "build release" task so kwxFFI/bin/Release matches the source.'
fi

PATH_KWXGEN=''
if command -v kwxgen >/dev/null 2>&1; then
    PATH_KWXGEN="$(command -v kwxgen)"
fi
if [ -z "$KWXGEN" ]; then
    KWXGEN="$PATH_KWXGEN"
fi
if [ -z "$KWXGEN" ]; then
    Say 'Error: kwxgen was not found.'
    Say '       Build the sibling kwxFFI repository (bin/Release) or set KWXGEN_EXE.'
    exit 1
fi

# Normalize to a path with no `..` before it is compared with `command -v kwxgen`
# below. The canonical artifact is usually also the PATH copy, and a string-only
# difference would otherwise report a stale mirror on every run.
if KWXGEN_DIR="$(cd "$(dirname "$KWXGEN")" >/dev/null 2>&1 && pwd_win)"; then
    KWXGEN="$KWXGEN_DIR/$(basename "$KWXGEN")"
fi

Say "Using kwxgen: $KWXGEN"
kwxgen_size="$(wc -c < "$KWXGEN" | tr -d ' ')"
kwxgen_modified="$(date -r "$KWXGEN" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || printf 'unknown')"
Say "  size $kwxgen_size bytes  modified $kwxgen_modified"

if [ -n "$PATH_KWXGEN" ] && [ "$PATH_KWXGEN" != "$KWXGEN" ]; then
    Say "Note: PATH also resolves kwxgen to $PATH_KWXGEN"
    Say '      The source-tree build above was used. Run the kwxFFI "build release" task so the'
    Say '      PATH copy matches it; manifest.json records kwxgen.sha256 for comparison.'
fi

# A binary older than the emitter source silently describes a previous build: every
# descriptor it writes looks well-formed, and only --verify catches the languages whose
# ctor naming changed. Warn instead of trusting the size/date above at face value.
KWXFFI_ROOT="$ROOT/../kwxFFI"
if [ -d "$KWXFFI_ROOT/kwxgen/src" ]; then
    newer_source="$(find "$KWXFFI_ROOT/kwxgen/src" -type f -newer "$KWXGEN" -print -quit 2>/dev/null)"
    if [ -n "$newer_source" ]; then
        Say "Warning: $KWXGEN predates $newer_source"
        Say '         Run the kwxFFI "build release" task before trusting these descriptors.'
    fi
fi

# The manifest writer reads this to record which binary wrote the descriptors.
export KWXGEN_BIN="$KWXGEN"

# Pre-flight: a kwxgen without --classes rejects the option; a current one rejects
# the missing --headers/--defs instead. Only the first means the descriptor will
# be silently narrowed.
if "$KWXGEN" emit-ui --classes auto 2>&1 | grep -q 'Unknown argument: --classes'; then
    Say "Error: $KWXGEN does not support 'emit-ui --classes'."
    Say '       That copy predates class selection, so it emits only the seven required classes'
    Say '       and hides every emitter gap.'
    Say '       Fix: run the kwxFFI "build release" task (repo root .vscode/tasks.json). It builds'
    Say '       bin/Release/kwxgen.exe and mirrors it to C:\rwCode\bin when the hashes differ.'
    Say '       Do not run the kwxgen-local "build release (no mirror)" task: it builds the same'
    Say '       project without mirroring, leaving this stale copy on PATH.'
    exit 1
fi

cd "$ROOT" || exit 1

# Class selection. "auto" describes every class in the profiled model, which is
# what the gap oracle needs. Set KWXGEN_CLASSES=form for the seven required
# classes only, or to a file path (one class name per line) to narrow a
# descriptor you intend to review by hand.
KWXGEN_CLASSES="${KWXGEN_CLASSES:-auto}"
Say "Class selection: $KWXGEN_CLASSES"

for lang in fortran go julia lua rust typescript; do
    Say "Generating src/generate/ffi_ui/$lang.json"
    if ! "$KWXGEN" emit-ui --headers ../kwxFFI/include --defs ../kwxFFI/src/kwx_defs.cpp \
        --lang "$lang" --classes "$KWXGEN_CLASSES" --verify \
        --out "src/generate/ffi_ui/$lang.json"; then
        Say "Error: kwxgen emit-ui failed for $lang"
        exit 1
    fi
done

if ! pwsh -NoProfile -ExecutionPolicy Bypass -File "$SCRIPT_DIR/gen_ffi_ui_manifest.ps1"; then
    Say 'Error: failed to write src/generate/ffi_ui/manifest.json'
    exit 1
fi

# verified.json is untracked curated evidence, so a fresh checkout has none.
# Seed a schema-valid empty stub so coverage.json generation and kwx_coverage.ps1
# can run; never overwrite curated evidence that is already present.
VERIFIED_JSON="$ROOT/src/generate/ffi_ui/verified.json"
if [ ! -f "$VERIFIED_JSON" ]; then
    printf '{"schema": 1, "entries": []}\n' > "$VERIFIED_JSON"
    Say 'Note: seeded src/generate/ffi_ui/verified.json (untracked; no verified-emitter evidence is present).'
fi

Say 'Descriptors and manifest.json are up to date.'
exit 0
