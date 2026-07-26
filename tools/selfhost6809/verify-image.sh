#!/usr/bin/env bash
# verify-image.sh <image> -- host-side structural assertions on a built
# conformance image, using ToolShed as an oracle independent of the
# emulator that built it.
set -uo pipefail
IMG="${1:?usage: verify-image.sh <image>}"
OS9=${OS9:-/usr/local/bin/os9}
fails=0
fail() { printf 'ASSERT-FAIL %s\n' "$*"; fails=$((fails+1)); }

[ -f "$IMG" ] || { printf 'ASSERT-FAIL image %s does not exist\n' "$IMG"; exit 1; }

listing=$("$OS9" dir -e "$IMG," 2>&1)

# Every required directory is present.
for d in CMDS SRC DOCS SCRATCH RESULTS; do
    printf '%s\n' "$listing" | grep -aq "[[:space:]]$d\$" || fail "missing directory $d"
done

# Nothing on the image is owned by group 0. Microware defines the super user
# as ANY user in group zero, so a group-0 owner would ship privileged files.
if printf '%s\n' "$listing" | grep -aqE '^[[:space:]]*0\.'; then
    fail "found group-0 (super user) ownership in root listing"
fi

## Assertions added in Task 4, once the files they check exist.
## Until then this script must not assert them: a check for something no task
## has created yet fails for the wrong reason and teaches you to ignore it.

# Every staged module actually landed on the image, byte-for-byte. The OS-9
# shell does not abort a build procedure on a failed command -- a `copy`
# whose source is missing prints an Error # line and the procedure runs to
# completion regardless, leaving the destination silently absent. The
# BUILD-STRUCTURE-DONE marker in build.log proves only that the procedure
# ran, never that each step worked, so this compares the build's own
# manifest.txt (written as it staged each module) against what actually
# exists in CMDS/ on the finished image.
MANIFEST="$(dirname "$IMG")/manifest.txt"
if [ -f "$MANIFEST" ]; then
    cmds=$("$OS9" dir -e "$IMG,/CMDS" 2>&1)
    while read -r name bytes; do
        [ -z "$name" ] && continue
        line=$(printf '%s\n' "$cmds" | grep -aE "[[:space:]]${name}\$") \
            || { fail "manifest entry $name missing from CMDS/ on image"; continue; }
        got=$(printf '%s\n' "$line" | awk '{print $(NF-1)}')
        [ "$got" = "$bytes" ] \
            || fail "manifest entry $name size mismatch: image has $got bytes, manifest says $bytes"
    done < "$MANIFEST"
else
    fail "manifest not found at $MANIFEST"
fi

# The build log itself must show no failed command. A silently-failed copy
# or attr is otherwise invisible: its test module is just missing from the
# suite's report, which reads as a shorter suite rather than as a fault.
BUILDLOG="$(dirname "$IMG")/build.log"
if [ -f "$BUILDLOG" ]; then
    grep -aE 'Error #' "$BUILDLOG" >/dev/null \
        && fail "build.log contains an Error # line -- a build step failed silently"
else
    fail "build log not found at $BUILDLOG"
fi

if [ "$fails" -eq 0 ]; then printf 'VERIFY-OK %s\n' "$IMG"; exit 0; fi
printf 'VERIFY-FAILED %d assertion(s)\n' "$fails"; exit 1
