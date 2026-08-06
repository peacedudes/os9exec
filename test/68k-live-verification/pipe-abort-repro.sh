#!/bin/sh
# Reproduce + verify the pSysTask keyboard-abort fix.
#
# A process blocked writing to a full named pipe (no reader) parks in os9exec's
# pSysTask state. Before the fix, that branch of the main loop never drained the
# async signal queue, so a queued Ctrl-C/Ctrl-E abort was never delivered -- the
# process wedged until a full restart. After the fix it is aborted normally.
#
# This needs a real timed keystroke (send the abort AFTER the write blocks), so
# it lives here as a runnable script rather than in the piped OS9Tests suite,
# which delivers all stdin at once and cannot time the abort.
#
# Usage:  test/68k-live-verification/pipe-abort-repro.sh
# Exit 0 = abort interrupted the blocked writer (fix present); 1 = wedged.
set -e

REPO=$(cd "$(dirname "$0")/../.." && pwd)
OS9EXEC="$REPO/os9exec"
DISK="${OS9DISK:?set OS9DISK to an OS-9 system disk}"
SCR=$(mktemp -d)
trap 'rm -rf "$SCR"' EXIT
TIMEOUT=$(command -v gtimeout || command -v timeout)

cp "$(dirname "$0")/pipe-abort-blk.c" "$SCR/blk.c"
flip -m "$SCR/blk.c"

# Build the blocker into the scratch device (never the system disk).
printf 'load /dd/CMDS/cpp /dd/CMDS/c68 /dd/CMDS/o68 /dd/CMDS/r68 /dd/CMDS/l68\nsetenv CLIB /dd/LIB\nsetenv CDEF /dd/DEFS\nchd /h5\ncc blk.c -F=/h5/blk\nstop\n' \
  | "$TIMEOUT" 60 env OS9STOP=1 OS9DISK="$DISK" OS9H5="$SCR" "$OS9EXEC" -r /dd/CMDS/shell >/dev/null 2>&1
[ -f "$SCR/blk" ] || { echo "FAIL: blk did not compile"; exit 1; }

# Run the blocker, let it wedge, then send Ctrl-C/Ctrl-E; a working abort lets
# the shell return and run the marker.
out=$( ( printf 'chd /h5\n/h5/blk\n'; sleep 3; printf '\003\005\003\005'; \
         sleep 2; printf 'echo REVIVED_MARKER\nstop\n'; sleep 2 ) \
       | "$TIMEOUT" 20 env OS9STOP=1 OS9DISK="$DISK" OS9H5="$SCR" "$OS9EXEC" -r /dd/CMDS/shell 2>&1 \
       | tr '\r' '\n' )

if printf '%s' "$out" | grep -q REVIVED_MARKER; then
    echo "PASS: Ctrl-C/Ctrl-E interrupted a process blocked on a full pipe write"
    exit 0
else
    echo "FAIL: blocked pipe writer ignored the abort (wedged) -- the pSysTask fix is missing"
    exit 1
fi
