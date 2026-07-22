#!/bin/sh
# Reproduce + verify E$BMode enforcement on write-to-a-read-only-opened file.
#
# os9exec used to ignore a path's open access mode: writing to a file opened
# read-only (and reading a file opened write-only) succeeded, where real OS-9
# RBF returns E$BMode (203). syspath_write now checks spP->mode for disk-file
# types (fRBF/fFile), exempting the shared console (fCons/fTTY) paths.
#
# Usage:  test/68k-live-verification/bmode-repro.sh
# Exit 0 = E$BMode enforced (fix present); 1 = write went through (fix missing).
set -e

REPO=$(cd "$(dirname "$0")/../.." && pwd)
# If the relative path didn't find os9exec (e.g., running from test Fixtures),
# walk up to find the repo root by looking for the os9exec binary itself.
if [ ! -f "$REPO/os9exec" ]; then
    REPO=$(dirname "$(dirname "$0")")
    while [ ! -f "$REPO/os9exec" ] && [ "$REPO" != "/" ]; do
        REPO=$(dirname "$REPO")
    done
fi
OS9EXEC="$REPO/os9exec"
DISK="$REPO/h0"
SCR=$(mktemp -d)
trap 'rm -rf "$SCR"' EXIT
TIMEOUT=$(command -v gtimeout || command -v timeout)

cp "$(dirname "$0")/bmode.c" "$SCR/bmode.c"
flip -m "$SCR/bmode.c"

out=$( printf 'load /dd/CMDS/cpp /dd/CMDS/c68 /dd/CMDS/o68 /dd/CMDS/r68 /dd/CMDS/l68\nsetenv CLIB /dd/LIB\nsetenv CDEF /dd/DEFS\nchd /h5\ncc bmode.c -F=/h5/bmode\n/h5/bmode\nstop\n' \
       | "$TIMEOUT" 60 env OS9STOP=1 OS9DISK="$DISK" OS9H5="$SCR" "$OS9EXEC" -r /dd/CMDS/shell 2>&1 | tr '\r' '\n' )

if printf '%s' "$out" | grep -q 'PASS E\$BMode'; then
    echo "PASS: write to a read-only-opened disk file is rejected with E\$BMode (203)"
    exit 0
else
    echo "FAIL: E\$BMode not enforced on write-to-read-only"
    printf '%s\n' "$out" | grep -aE 'FAIL|errno' | head
    exit 1
fi
