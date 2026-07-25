#!/usr/bin/env bash
# build-image.sh -- build the self-hosted 6809 conformance image.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$REPO/build/selfhost6809"
STAGE="$OUT/stage"          # host-native tree os9exec sees as /h1
IMGDIR="$OUT/dev"           # os9exec startPath: the image appears here as h7
ACCT=${CONF_ACCT:-claude}   # non-super account in h0/SYS/password (1.7)

rm -rf "$OUT"; mkdir -p "$STAGE" "$IMGDIR"
ln -s "$REPO/h0" "$IMGDIR/h0"
ln -s "$STAGE"   "$IMGDIR/h1"

# --- the os9exec build procedure, CR-only (LF would make it ONE line) ---
{
  echo "mount -k=360k h7"
  echo "login $ACCT"
  for d in CMDS SRC DOCS SCRATCH RESULTS; do echo "makdir /h7/$d"; done
  echo "echo BUILD-STRUCTURE-DONE"
  echo "dir -e /h7"
} | tr '\n' '\r' > "$STAGE/mkimg"

cd "$IMGDIR"
OS9DISK="$IMGDIR/h0" OS9STOP=1 "$REPO/os9exec" shell /h1/mkimg </dev/null 2>&1 \
  | grep -av '^#' | tee "$OUT/build.log"

grep -aq BUILD-STRUCTURE-DONE "$OUT/build.log" \
  || { echo "build: os9exec procedure did not complete" >&2; exit 1; }

mv "$IMGDIR/h7" "$OUT/conf6809.dsk"
printf 'built %s\n' "$OUT/conf6809.dsk"
