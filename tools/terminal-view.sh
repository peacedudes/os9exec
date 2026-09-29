#!/usr/bin/env bash
# terminal-view.sh -- what a user SEES when they run the conformance disk.
#
# Runs conf68k.dsk's shell-less runner with this tree's ./os9exec on a
# pseudo-terminal (script(1)), RUNS times from a fresh copy of the disk, and
# fails unless every screen ends in tally's "Nothing to send" with no line
# written over another.
#
# Every other check captures output through a pipe, where a line that ends
# in a bare carriage return still looks like a line of its own. On a
# terminal it sends the cursor back to the start of the SAME line, and the
# next line overwrites it. v4.1.0 shipped that way (run and tally wrote
# their CRs with I$Write, which gets no line feed), and nothing noticed.
# tools/release-check.sh makes the same check on the published binaries.
set -u
REPO="$(cd "$(dirname "$0")/.." && pwd)"
DSK="${1:-$REPO/build/selfhost68k/conf68k.dsk}"
RUNS=${RUNS:-3}
[ -s "$DSK" ] || { echo "no disk at $DSK (tools/selfhost68k/build-image.sh makes it)"; exit 1; }
W=$(mktemp -d "${TMPDIR:-/tmp}/termview-XXXXXX")
trap 'rm -rf "$W"' EXIT
rc=0
for i in $(seq 1 "$RUNS"); do
    cp "$DSK" "$W/c.dsk"
    script -q /dev/null env OS9DISK="$W/c.dsk" "$REPO/os9exec" -r /dd/CMDS/run </dev/null >"$W/screen" 2>&1
    if ! grep -q "Nothing to send" "$W/screen"; then
        echo "run $i: no all-clear:"; tr '\r' '\n' <"$W/screen" | tail -4; rc=1
    elif python3 -c 'import re,sys; sys.exit(0 if re.search(rb"\r[^\r\n]",open(sys.argv[1],"rb").read()) else 1)' "$W/screen"; then
        echo "run $i: a line of the verdict is written over another (bare CR):"
        od -c "$W/screen" | grep -m3 '\\r   [^\\]'; rc=1
    else
        echo "run $i: ok"
    fi
done
exit $rc
