#!/usr/bin/env bash
# upload.sh <host.bas> <guestname.bas> <procname>
#
# Uploads a BASIC09 source file to the guest and packs it into a module in the
# current execution directory.
#
# Uses `build` rather than `tee`: `tee >file` silently never started here, so
# every typed line fell through to the shell and was executed as a command
# (a flood of "Error #216" and "WHAT?", ending in a stray `bye` that logged the
# session out). `build` is the route b09run.sh already relies on.
#
# `build` terminates on a blank line, so the source must contain none.
#
# Every step is verified: a dropped line otherwise yields a plausible-looking
# module that quietly does the wrong thing, which is exactly how a previous
# session ended up with fixtures that matched no source in the repo.
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
REPL="${R:-$REPO/tools/nitros9repl.sh}"

host="$1"; guest="$2"; proc="$3"

if grep -qn '^[[:space:]]*$' "$host"; then
    echo "REFUSING: $host contains a blank line, which terminates \`build\`" >&2
    grep -n '^[[:space:]]*$' "$host" >&2
    exit 1
fi

type_line() { "$REPL" key "$1" Enter >/dev/null 2>&1; sleep 0.25; }

"$REPL" send "del $guest" >/dev/null 2>&1 || true
"$REPL" key "build $guest" Enter >/dev/null 2>&1
sleep 1.2

while IFS= read -r line || [ -n "$line" ]; do
    type_line "$line"
done < "$host"

"$REPL" key Enter >/dev/null 2>&1   # blank line terminates build
sleep 1.5

# Prove the guest copy matches the host source before packing it.
listing=$("$REPL" send "list $guest" 2>&1 || true)
for probe in "PROCEDURE $proc" "$(grep -m1 'v7' "$host" | sed 's/.*"\(.*\)".*/\1/' || true)"; do
    [ -n "$probe" ] || continue
    if ! printf '%s' "$listing" | grep -qF "$probe"; then
        echo "UPLOAD FAIL: $guest missing expected line: $probe" >&2
        printf '%s\n' "$listing" >&2
        exit 1
    fi
done
echo "verified upload: $guest"

# `pack` prompts "Rewrite?:" when the module already exists and eats the next
# script line as the answer, desyncing everything after it.  Delete first.
"$REPL" send "del $proc" >/dev/null 2>&1 || true
"$REPL" send "del $proc.pk" >/dev/null 2>&1 || true

"$REPL" key "build $proc.pk" Enter >/dev/null 2>&1
sleep 1.2
type_line "load $guest"
type_line "pack $proc"
type_line "bye"
"$REPL" key Enter >/dev/null 2>&1
sleep 1.5

"$REPL" send "basic09 #32k <$proc.pk" 2>&1 | tail -6

# A module that did not get created is the failure that looks most like success.
if ! "$REPL" send "dir" 2>&1 | tr -s ' ' '\n' | grep -qx "$proc"; then
    echo "PACK FAIL: module $proc not present after pack" >&2
    exit 1
fi
echo "packed module: $proc"
