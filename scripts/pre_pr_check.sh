#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# pre_pr_check.sh -- run everything in .github/workflows/pr_verify.yml EXCEPT
# the build jobs, locally and *before* a PR is opened.
#
#   job in pr_verify.yml    what runs here
#   ---------------------   -----------------------------------------------
#   clangFormat             clang-format 20 over ./src (the version CI pins);
#                           one diagnostic per file, not per changed column
#   check-spelling          typos
#   check-whitespace        git diff --check against the base commit
#   check-eol               LF-only line endings
#   buildUnix/buildMacOS    intentionally NOT run
#
# Why clang-format 20 and not whatever is on PATH: CI pins clangFormatVersion
# 20, and clang-format 21 reformats code that 20 accepts.  A different major
# version therefore produces false positives (and false negatives).  The
# script uses clang-format-20 from PATH when available, otherwise it runs
# clang-format 20 out of the DoozyX action image that `act` already built
# for this repository -- no network access needed.
#
# Usage: scripts/pre_pr_check.sh [options]
#
#   -b, --base <ref>   base ref for the diff based checks
#                      (default: merge-base of origin/main and HEAD)
#   -c, --changed      only inspect files changed against the base
#                      (default: every tracked file, exactly like CI does)
#   -f, --fix          reformat the offending files instead of only reporting
#       --no-docker    never fall back to Docker for clang-format 20
#       --only <list>  comma separated subset of: format,typos,whitespace,eol
#   -h, --help         this text
#
# Environment:
#   CLANG_FORMAT_20=<path>    use this clang-format 20.x executable
#   PRCHECK_CF_IMAGE=<image>  DoozyX action image (`act -j clangFormat` builds
#                             it as act-doozyx-clang-format-lint-action-v0-20-dockeraction)
#
# Exit status: 0 = every check passed, 1 = at least one check failed,
#              2 = the script itself could not run a check.
# ---------------------------------------------------------------------------
set -uo pipefail

# ------------------------------------------------------------------ helpers
if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
    C_RED=$'\033[31m'; C_GRN=$'\033[32m'; C_YEL=$'\033[33m'
    C_BLD=$'\033[1m'; C_DIM=$'\033[2m'; C_RST=$'\033[0m'
else
    C_RED=''; C_GRN=''; C_YEL=''; C_BLD=''; C_DIM=''; C_RST=''
fi

have() { command -v "$1" >/dev/null 2>&1; }

die() {
    printf '%s\n' "${C_RED}error: $*${C_RST}" >&2
    exit 2
}

step() { printf '\n%s\n' "${C_BLD}==> $*${C_RST}"; }
note() { printf '%s\n' "${C_DIM}    $*${C_RST}"; }
warn() { printf '%s\n' "${C_YEL}    warning: $*${C_RST}"; }

RESULTS=()
FAILED=0

# record <label> <0|1> [detail]
record() {
    local label=$1 status=$2 detail=${3:-} m pad
    printf -v pad '%-16s' "$label"
    if [ "$status" -eq 0 ]; then
        m="${pad}${C_GRN}ok${C_RST}"
    else
        FAILED=1
        m="${pad}${C_RED}FAIL${C_RST}"
        [ -n "$detail" ] && m="${m}  (${detail})"
    fi
    RESULTS+=("$m")
}

usage() {
    awk 'NR == 1 { next }
         /^#/ { line = $0; sub(/^# ?/, "", line)
                if (line ~ /^-+$/) next
                print line; next }
         { exit }' "$0"
    exit 0
}

# ------------------------------------------------------------------- options
BASE_ARG=''
CHANGED=0
FIX=0
USE_DOCKER=1
ONLY=''

while [ $# -gt 0 ]; do
    case "$1" in
        -b|--base)
            [ $# -ge 2 ] || die "--base needs a ref"
            BASE_ARG=$2; shift 2 ;;
        -c|--changed) CHANGED=1; shift ;;
        -f|--fix)     FIX=1; shift ;;
        --no-docker)  USE_DOCKER=0; shift ;;
        --only)
            [ $# -ge 2 ] || die "--only needs a list"
            ONLY=$2; shift 2 ;;
        -h|--help)    usage ;;
        *)            die "unknown option: $1  (try --help)" ;;
    esac
done

want() {  # want <format|typos|whitespace|eol>
    [ -z "$ONLY" ] && return 0
    case ",$ONLY," in
        *",$1,"*) return 0 ;;
        *) return 1 ;;
    esac
}

# ------------------------------------------------------------------ repo/ref
ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || die "not inside a git repository"
cd "$ROOT" || die "cannot enter $ROOT"

resolve_base() {
    local ref
    if [ -n "$BASE_ARG" ]; then
        printf '%s' "$BASE_ARG"
        return 0
    fi
    for ref in origin/main main; do
        if git rev-parse --verify --quiet "$ref^{commit}" >/dev/null 2>&1; then
            git merge-base "$ref" HEAD 2>/dev/null || git rev-parse "$ref"
            return 0
        fi
    done
    printf 'HEAD'
}

BASE=$(resolve_base)
git rev-parse --verify --quiet "$BASE^{commit}" >/dev/null 2>&1 ||
    die "base ref '$BASE' not found -- pass --base <ref>"

printf '%s\n' "${C_BLD}${0##*/} pre-PR check${C_RST}"
note "repository: $ROOT"
note "base:       $BASE ($(git rev-parse --short "$BASE" 2>/dev/null))"
note "scope:      $([ "$CHANGED" -eq 1 ] && echo 'files changed vs base' || echo 'all tracked files (same as CI)')"

# mktemp is unreliable here (TMPDIR may point at a directory sill removes), so
# keep the scratch files inside the git dir.
GIT_DIR=$(git rev-parse --git-dir 2>/dev/null) || GIT_DIR='.git'
TMP="$GIT_DIR/pre_pr_check.$$.1.tmp"
TMP2="$GIT_DIR/pre_pr_check.$$.2.tmp"
TMP3="$GIT_DIR/pre_pr_check.$$.3.tmp"
: > "$TMP" || die "cannot write $TMP"
: > "$TMP2" || die "cannot write $TMP2"
: > "$TMP3" || die "cannot write $TMP3"
trap 'rm -f "$TMP" "$TMP2" "$TMP3"' EXIT

# Pathspecs excluded by .clang-format-ignore -- parsed at run time so that the
# local check keeps following CI if that file changes.
EXCLUDES=()
if [ -f .clang-format-ignore ]; then
    while IFS= read -r ln || [ -n "$ln" ]; do
        ln=${ln%$'\r'}
        case "$ln" in ''|'#'*) continue ;; esac
        ln=${ln#./}
        EXCLUDES+=(":(exclude)${ln%/}")
    done < .clang-format-ignore
fi

CF_SPEC=('src/*.h' 'src/*.cpp' "${EXCLUDES[@]}")

list_cf_files() {
    if [ "$CHANGED" -eq 1 ]; then
        git diff --name-only --diff-filter=ACMR "$BASE" -- "${CF_SPEC[@]}"
    else
        git ls-files -- "${CF_SPEC[@]}"
    fi
    # files not yet added to git are invisible to the commands above
    git ls-files --others --exclude-standard -- "${CF_SPEC[@]}"
}

# ------------------------------------------------------- 1. clang-format 20
CF_IMAGE=${PRCHECK_CF_IMAGE:-act-doozyx-clang-format-lint-action-v0-20-dockeraction:latest}

cf_native_bin() {
    local cand v
    for cand in "${CLANG_FORMAT_20:-}" clang-format-20 clang-format20 clang-format; do
        [ -n "$cand" ] || continue
        have "$cand" || continue
        v=$("$cand" --version 2>/dev/null) || continue
        case "$v" in
            *"clang-format version 20."*) printf '%s' "$cand"; return 0 ;;
        esac
    done
    return 1
}

CF_BACKEND=''   # native | docker
CF_BIN=''

cf_invoke() {  # cf_invoke <files...>
    local flags=(--dry-run -Werror --style=file)
    [ "$FIX" -eq 1 ] && flags=(-i --style=file)
    if [ "$CF_BACKEND" = native ]; then
        "$CF_BIN" "${flags[@]}" "$@"
    else
        docker run --rm -v "$ROOT:/src" -w /src \
            --entrypoint /clang-format/clang-format20 \
            "$CF_IMAGE" "${flags[@]}" "$@"
    fi
}

# clang-format reports one diagnostic per *column* that would move, and prints
# an echoed source line plus a caret line under each one.  Left as-is that
# turns the Problems panel into a wall of near-identical entries for a single
# file.  Collapse it to one line per file -- all that is needed to know the
# file has to be reformatted:
#
#   path:line:col: error: <n> formatting change(s) needed (clang-format 20)
#
# line/col is the first offending position, so clicking the entry jumps to it.
# Anything that is not a -Wclang-format-violations diagnostic (a real parse
# error, a failed docker run) is passed through untouched.
CF_CUR=''      # file the pending diagnostics belong to
CF_N=0         # number of diagnostics for CF_CUR
CF_LINE=0      # first offending position in CF_CUR
CF_COL=0
CF_STATE=0     # 0 = normal, 1 = expect echoed source line, 2 = expect caret line

cf_flush() {
    [ -n "$CF_CUR" ] || return 0
    printf '%s:%s:%s: error: %s formatting change(s) needed (clang-format 20)\n' \
        "$CF_CUR" "$CF_LINE" "$CF_COL" "$CF_N"
    CF_CUR=''
}

# Split "path:line:col: <severity>: code should be clang-formatted ..." into
# CF_PATH/CF_LNUM/CF_CNUM.  Plain parameter expansion rather than [[ =~ ]] so
# the script runs under any bash-like shell (sill/brush, dash, ...).
cf_split_diag() {   # returns 1 when the line is not a -Wclang-format-violations diagnostic
    local rest=$1 sev
    case $rest in
        *': error: code should be clang-formatted'*)   sev=': error: code should be clang-formatted' ;;
        *': warning: code should be clang-formatted'*) sev=': warning: code should be clang-formatted' ;;
        *) return 1 ;;
    esac
    rest=${rest%%"$sev"*}            # "path:line:col"
    CF_CNUM=${rest##*:}
    rest=${rest%:*}
    CF_LNUM=${rest##*:}
    CF_PATH=${rest%:*}
    return 0
}

cf_collapse() {
    local line t
    while IFS= read -r line || [ -n "$line" ]; do
        if cf_split_diag "$line"; then
            if [ "$CF_PATH" != "$CF_CUR" ]; then
                cf_flush
                CF_CUR=$CF_PATH
                CF_LINE=$CF_LNUM
                CF_COL=$CF_CNUM
                CF_N=0
            fi
            CF_N=$((CF_N + 1))
            CF_STATE=1
            continue
        fi
        case $CF_STATE in
            1) CF_STATE=2; continue ;;          # echoed source line
            2)
                CF_STATE=0
                t=${line#"${line%%[![:space:]]*}"}
                case $t in
                    '^'*) continue ;;                # caret line
                esac
                ;;
        esac
        cf_flush
        printf '%s\n' "$line"
    done
    cf_flush
}

cf_run() {  # cf_run <files...>
    local status=0 f
    local chunk=()
    for f in "$@"; do
        chunk+=("$f")
        # keep each command line well under the Windows 32K limit
        if [ ${#chunk[@]} -ge 100 ]; then
            cf_invoke "${chunk[@]}" 2>&1 | cf_collapse || status=1
            chunk=()
        fi
    done
    if [ ${#chunk[@]} -gt 0 ]; then
        cf_invoke "${chunk[@]}" 2>&1 | cf_collapse || status=1
    fi
    return $status
}

check_format() {
    step "clangFormat -- clang-format 20 (as pinned by CI)"
    local count=0 status=0 f
    local files=()

    list_cf_files > "$TMP"
    while IFS= read -r f; do files+=("$f"); done < "$TMP"
    count=${#files[@]}
    if [ "$count" -eq 0 ]; then
        note "no files to check"
        record clang-format 0 ''
        return 0
    fi
    note "$count file(s) to check"

    if CF_BIN=$(cf_native_bin); then
        CF_BACKEND=native
        note "using $CF_BIN ($("$CF_BIN" --version 2>/dev/null))"
    elif [ "$USE_DOCKER" -eq 1 ] && have docker &&
         docker image inspect "$CF_IMAGE" >/dev/null 2>&1; then
        CF_BACKEND=docker
        note "using clang-format 20 from image $CF_IMAGE"
    else
        warn "no clang-format 20 found and Docker image $CF_IMAGE is unavailable."
        warn "install one with:  pip install clang-format==20.1.8"
        warn "or build the act image with:  act -j clangFormat"
        record clang-format 1 'clang-format 20 not available'
        return 1
    fi

    cf_run "${files[@]}" > "$TMP3" 2>&1 || status=1
    cat "$TMP3"

    if [ "$status" -eq 0 ]; then
        record clang-format 0 ''
    elif [ "$FIX" -eq 1 ]; then
        record clang-format 0 'reformatted'
    else
        local l nfiles=0
        while IFS= read -r l; do
            case $l in
                *'formatting change(s) needed (clang-format 20)') nfiles=$((nfiles + 1)) ;;
            esac
        done < "$TMP3"
        if [ "$nfiles" -gt 0 ]; then
            record clang-format 1 "formatting violations in $nfiles file(s)"
        else
            record clang-format 1 'clang-format failed'
        fi
        note "fix with: scripts/pre_pr_check.sh --fix --only format"
    fi
    return $status
}

# ------------------------------------------------------------ 2. check-spelling
check_typos() {
    step "check-spelling -- typos"
    if ! have typos; then
        warn "typos is not installed (https://github.com/crate-ci/typos)"
        record spelling 1 'typos not installed'
        return 1
    fi
    note "$(typos --version 2>/dev/null)"

    local status=0
    if [ "$CHANGED" -eq 1 ]; then
        local files=() f
        {
            git diff --name-only --diff-filter=ACMR "$BASE"
            git ls-files --others --exclude-standard
        } > "$TMP"
        while IFS= read -r f; do
            [ -f "$f" ] && files+=("$f")
        done < "$TMP"
        if [ ${#files[@]} -eq 0 ]; then
            note "no files to check"
            record spelling 0 ''
            return 0
        fi
        # typos applies [files] extend-exclude only to the directory walk; a path
        # named on the command line is checked even when it matches an exclude.
        # --changed passes an explicit list, so without --force-exclude a modified
        # file under scripts/ (or 3rdparty/, frozen/, *.wxui, ...) would be spell
        # checked here although CI's whole-repo walk skips it.
        typos --force-exclude "${files[@]}" || status=$?
    else
        typos . || status=$?
    fi

    if [ "$status" -eq 0 ]; then
        record spelling 0 ''
    elif [ "$status" -eq 2 ]; then
        record spelling 1 'typos found'
    else
        record spelling 1 "typos exited $status"
    fi
    return $status
}

# ---------------------------------------------------------- 3. check-whitespace
# Path list copied from the workflow so both sides agree on what is inspected.
WS_SPEC=(
    'src/*.cpp' 'src/*.h' 'src/customprops/' 'src/custom_ctrls/' 'src/generate/'
    'src/import/' 'src/internal/' 'src/mockup/' 'src/newdialogs/' 'src/nodes/'
    'src/panels/' 'src/project/' 'src/tests/' 'src/tools/' 'src/ui/' 'src/utils/'
    'src/winres/' 'src/wxue_namespace/' 'src/wxui/' ':(exclude)src/pugixml/'
)

# git diff --check never looks at untracked files, but CI will see them once
# they are committed -- so scan those separately with the same rules.
check_untracked_whitespace() {
    local status=0 f n=0 line indent
    git ls-files --others --exclude-standard -- "${WS_SPEC[@]}" > "$TMP"
    while IFS= read -r f; do
        n=0
        while IFS= read -r line || [ -n "$line" ]; do
            n=$((n + 1))
            case "$line" in
                *' '|*$'\t')
                    printf '%s:%s: trailing whitespace\n' "$f" "$n"
                    status=1 ;;
            esac
            indent=${line%%[![:blank:]]*}
            case "$indent" in
                *$'\t'*)
                    printf '%s:%s: tab in indent\n' "$f" "$n"
                    status=1 ;;
            esac
        done < "$f"
    done < "$TMP"
    return $status
}

check_whitespace() {
    step "check-whitespace -- git diff --check against $BASE"
    local status=0
    git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol,tab-in-indent \
        diff --check "$BASE" -- "${WS_SPEC[@]}" || status=$?
    if [ "$status" -ne 0 ] && [ "$status" -ne 2 ]; then
        record whitespace 1 "git diff exited $status"
        return $status
    fi
    check_untracked_whitespace || status=1
    [ "$status" -eq 2 ] && status=1
    if [ "$status" -eq 0 ]; then
        record whitespace 0 ''
    else
        record whitespace 1 'whitespace errors'
    fi
    return $status
}

# ---------------------------------------------------------------- 4. check-eol
# Mirrors scripts/check_eol.sh (src/*.cpp,*.h,*.cmake,*.xml minus src/pugixml,
# plus .github/*.yml) but reads the bytes git actually has on disk.
check_eol() {
    step "check-eol -- LF-only line endings"
    local status=0 line cols path we f
    git ls-files --eol -- 'src/' ':(exclude)src/pugixml/' '.github/' > "$TMP"
    if [ ! -s "$TMP" ]; then
        record eol 1 'git ls-files --eol produced nothing'
        return 1
    fi
    while IFS= read -r line; do
        cols=${line%%$'\t'*}
        path=${line#*$'\t'}
        case "$path" in
            *.cpp|*.h|*.cmake|*.xml|.github/*.yml) ;;
            *) continue ;;
        esac
        set -- $cols
        we=${2:-}
        case "$we" in
            w/lf|w/-text|w/none) ;;
            '')
                warn "unparsed eol info for $path: $line"
                status=1 ;;
            *)
                printf '%s\n' "${C_RED}non-LF line endings (${we}): ${path}${C_RST}"
                status=1 ;;
        esac
    done < "$TMP"

    # `git ls-files --eol` only knows about tracked files, so scan the files that
    # are still untracked -- CI will see them as soon as they are committed.
    git ls-files --others --exclude-standard -- 'src/' ':(exclude)src/pugixml/' '.github/' > "$TMP2"
    while IFS= read -r f; do
        case "$f" in
            *.cpp|*.h|*.cmake|*.xml|.github/*.yml) ;;
            *) continue ;;
        esac
        while IFS= read -r line || [ -n "$line" ]; do
            case "$line" in
                *$'\r')
                    printf '%s\n' "${C_RED}non-LF line endings (CR): ${f}${C_RST}"
                    status=1
                    break ;;
            esac
        done < "$f"
    done < "$TMP2"

    if [ "$status" -eq 0 ]; then
        record eol 0 ''
    else
        record eol 1 'CRLF/CR found'
    fi
    return $status
}

# ---------------------------------------------------------------------- main
want format     && check_format
want typos      && check_typos
want whitespace && check_whitespace
want eol        && check_eol

printf '\n%s\n' "${C_BLD}==> summary${C_RST}"
for r in "${RESULTS[@]}"; do
    printf '   %s\n' "$r"
done

if [ "$FAILED" -eq 0 ]; then
    printf '\n%s\n' "${C_GRN}all checks passed${C_RST}"
    exit 0
fi
printf '\n%s\n' "${C_RED}at least one check failed${C_RST}"
exit 1
