#!/usr/bin/env bash
# run-image.sh -- run the self-contained CONF68K disk the way an operator would.
#
# Boots the suite straight off the image with no shell and no system disk, then
# prints the totals. This is the "does it just work" entry point; use
# verify-image.sh when you want the contents checked as well.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$REPO/build/selfhost68k"
IMG="${1:-$OUT/conf68k.dsk}"
SUITE="$REPO/test/68k-conformance"
EXE="$REPO/os9exec"
TIMEOUT=$(command -v gtimeout || command -v timeout)

[ -f "$IMG" ] || { echo "no image at $IMG -- run tools/selfhost68k/build-image.sh" >&2; exit 2; }

echo "== CONF68K, self-contained disk: $(basename "$IMG") =="
printf 'RUN prebuilt\r' > "$SUITE/RESULTS/report"
for m in $(ls "$SUITE/CMDS" | grep -vE '^(tally|mark)$' | sort); do
    # cd to the image's own directory -- see verify-image.sh for why.
    line=$( cd "$(dirname "$IMG")" && $TIMEOUT 60 env OS9DISK="$IMG" "$EXE" \
            -r "/dd/CMDS/$m" </dev/null 2>&1 | tr '\r' '\n' | grep -a '^RESULT ' )
    [ -n "$line" ] && printf '%s\r' "$line" >> "$SUITE/RESULTS/report"
done

tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -a '^RESULT ' | grep -v ' PASS ' | sed 's/^/  /'
printf '  PASS=%s FAIL=%s SKIP=%s ERROR=%s\n' \
  "$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' PASS ')" \
  "$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' FAIL ')" \
  "$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' SKIP ')" \
  "$(tr '\r' '\n' < "$SUITE/RESULTS/report" | grep -c ' ERROR ')"
