#!/bin/bash

# Check that all source files use Unix (LF) line endings.
# This repository requires LF line endings for ALL files.

cd "$(dirname "$0")/.."

rc=0

# Find all text files in src/, excluding third-party code
find src \( \
    -path 'src/pugixml' -prune -o \
    -path 'src/frozen' -prune \) \
    -o -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.cmake' -o -name '*.xml' \) -print0 |
while IFS= read -r -d '' file; do
    # Get line ending counts: DOS UNIX MAC
    read dos unix mac <<< $(dos2unix --info=dum "$file" | awk '{print $1, $2, $3}')

    # Check for any DOS (CRLF) or Mac (CR) line endings
    if [[ "$dos" -gt 0 ]] || [[ "$mac" -gt 0 ]]; then
        echo "ERROR - non-LF line endings: $file (DOS=$dos, Unix=$unix, Mac=$mac)"
        rc=$((rc+1))
    fi
done

# Also check workflow files
find .github -type f -name '*.yml' -print0 |
while IFS= read -r -d '' file; do
    read dos unix mac <<< $(dos2unix --info=dum "$file" | awk '{print $1, $2, $3}')
    if [[ "$dos" -gt 0 ]] || [[ "$mac" -gt 0 ]]; then
        echo "ERROR - non-LF line endings: $file (DOS=$dos, Unix=$unix, Mac=$mac)"
        rc=$((rc+1))
    fi
done

exit $rc
