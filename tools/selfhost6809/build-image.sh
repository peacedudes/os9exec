#!/usr/bin/env bash
# build-image.sh -- build the self-hosted 6809 conformance image.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$REPO/build/selfhost6809"
STAGE="$OUT/stage"          # host-native tree os9exec sees as /h1
IMGDIR="$OUT/dev"           # os9exec startPath: the image appears here as h7
ACCT=${CONF_ACCT:-claude}   # non-super account in h0/SYS/password (1.7)

rm -rf "$OUT"; mkdir -p "$STAGE" "$IMGDIR" "$STAGE/CMDS" "$STAGE/SRC"
ln -s "$REPO/h0" "$IMGDIR/h0"
ln -s "$STAGE"   "$IMGDIR/h1"

TESTSRC="$REPO/test/6809-conformance/SRC"
MANIFEST="$OUT/manifest.txt"

# --- assemble every shipped test, stage the shippable sources ---
# SRC/ contains only files that ship (dev/ scaffolding never does); both
# loops below wildcard over it so a new test needs no build-script edit.
# manifest.txt records "<name> <bytes>" per module -- verify-image.sh uses
# it to prove each one actually landed on the image, not just that the
# build procedure ran (a failed `copy` does not abort the procedure; see
# task-3-report.md fix round 1).
: > "$MANIFEST"
for a in "$TESTSRC"/*.a; do
  b=$(basename "$a" .a)
  lwasm --format=os9 -I "$TESTSRC" --output="$STAGE/CMDS/$b" "$a"
  bytes=$(wc -c < "$STAGE/CMDS/$b" | tr -d ' ')
  echo "$b $bytes" >> "$MANIFEST"
done
for s in "$TESTSRC"/*; do
  tr '\n' '\r' < "$s" > "$STAGE/SRC/$(basename "$s")"   # OS-9 text is CR-only
done

# --- the os9exec build procedure, CR-only (LF would make it ONE line) ---
{
  echo "mount -k=360k h7"
  echo "login $ACCT"
  for d in CMDS SRC DOCS SCRATCH RESULTS; do echo "makdir /h7/$d"; done
  for m in "$STAGE"/CMDS/*; do
    b=$(basename "$m")
    # -n: create a fresh destination FD instead of replaying the source's
    # whole FD sector. Host-mounted /h1 synthesizes owner 0.0 (superuser)
    # for every file it has no real OS-9 ownership concept for; without -n,
    # copy tries to replicate that owner onto the new file on /h7 via one
    # SS.FD PutStat, which os9exec's RBF handler refuses for a non-super
    # caller whose real owner differs from 0 (E$PERMIT, 164) -- the file's
    # bytes are already written by then, so the copy still "worked", but
    # build.log gained a spurious Error # line. -n sidesteps the whole-FD
    # replication; the explicit attr below sets what the module needs.
    echo "copy -n /h1/CMDS/$b /h7/CMDS/$b"
    echo "attr /h7/CMDS/$b -e -pe -pr"    # 68k spelling: -e SETS, -ne clears
  done
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
