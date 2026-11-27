#!/bin/bash

# Check that all source files use Unix (LF) line endings.
# This repository requires LF line endings for ALL committed files.
#
# Files are enumerated through git (tracked files plus untracked files that are
# not ignored), which is what CI sees after a checkout. Generated and ignored
# files (for example src/version.h) are not checked.

cd "$(dirname "$0")/.." || exit 1

if ! command -v dos2unix >/dev/null 2>&1; then
    echo "ERROR - dos2unix is required (apt-get install dos2unix)" >&2
    exit 1
fi

rc=0
checked=0

check_file() {
    local file=$1 dos unix mac info

    # Output format is "<DOS> <UNIX> <MAC>  <filename>"
    if ! info=$(dos2unix --info=dum "$file" 2>/dev/null); then
        echo "ERROR - cannot read line-ending info: $file"
        rc=$((rc+1))
        return
    fi

    read -r dos unix mac _ <<< "$info"
    if [[ -z $dos || -z $unix || -z $mac ]]; then
        echo "ERROR - unexpected dos2unix output for $file: $info"
        rc=$((rc+1))
        return
    fi

    checked=$((checked+1))

    # Check for any DOS (CRLF) or Mac (CR) line endings
    if [[ $dos -gt 0 || $mac -gt 0 ]]; then
        echo "ERROR - non-LF line endings: $file (DOS=$dos, Unix=$unix, Mac=$mac)"
        rc=$((rc+1))
    fi
}

# Enumerate into a temp file: a pipeline would hide git's exit status, and an
# error there must not turn into a silent "everything is fine" pass.
file_list=$(mktemp) || exit 1
trap 'rm -f "$file_list"' EXIT

if ! git ls-files -z -c -o --exclude-standard \
    -- 'src/' ':(exclude)src/pugixml/' '.github/' > "$file_list"; then
    echo "ERROR - git ls-files failed while enumerating the source tree" >&2
    exit 1
fi

while IFS= read -r -d '' file; do
    # src/ sources (third-party pugixml excluded above) and workflow files
    case $file in
        src/*.cpp | src/*.h | src/*.cmake | src/*.xml | .github/*.yml) ;;
        *) continue ;;
    esac
    # git also reports index entries that were deleted from the working tree
    [[ -f $file ]] || continue
    check_file "$file"
done < "$file_list"

if [[ $checked -eq 0 ]]; then
    echo "ERROR - no files were checked" >&2
    exit 1
fi

if [[ $rc -ne 0 ]]; then
    echo "FAILED - $rc file(s) with non-LF line endings (checked $checked)"
    exit 1
fi

echo "OK - $checked file(s) checked, all use LF line endings"
exit 0
