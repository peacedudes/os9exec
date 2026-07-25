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

if [ "$fails" -eq 0 ]; then printf 'VERIFY-OK %s\n' "$IMG"; exit 0; fi
printf 'VERIFY-FAILED %d assertion(s)\n' "$fails"; exit 1
