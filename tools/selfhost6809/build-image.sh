#!/usr/bin/env bash
# build-image.sh -- build the self-hosted 6809 conformance image.
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$REPO/build/selfhost6809"
STAGE="$OUT/stage"          # host-native tree os9exec sees as /h1
IMGDIR="$OUT/dev"           # os9exec startPath: the image appears here as h7
ACCT=${CONF_ACCT:-claude}   # non-super account in h0/SYS/password (1.7)

rm -rf "$OUT"; mkdir -p "$STAGE" "$IMGDIR" "$STAGE/CMDS" "$STAGE/SRC" "$STAGE/SCRATCH"
ln -s "${OS9DISK:?set OS9DISK to the system disk}" "$IMGDIR/h0"
ln -s "$STAGE"   "$IMGDIR/h1"

# t05fna's fixture: a file that exists but that t05fna must not be able to
# open. Content is irrelevant -- the test never reads it -- only that it
# exists and, once copied onto the image below with -npr, is owned by the
# build account with no public read. CR-only, matching every other staged
# text file's convention on this image.
printf 'This file backs test t05fna. It exists only to be denied.\n' \
  | tr '\n' '\r' > "$STAGE/SCRATCH/denied"

TESTSRC="$REPO/test/6809-conformance/SRC"
TEXTSRC="$REPO/test/6809-conformance/text"
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
# BASIC09 sources have no host-side compiler (see task-4-report.md): the
# packed module for each SRC/<name>.bas is committed alongside it as
# SRC/<name>.pack, built once on a live NitrOS-9 guest. Installed raw --
# packing already happened when the .pack file was committed, so this needs
# no guest and no XRoar for an ordinary build.
for p in "$TESTSRC"/*.pack; do
  [ -e "$p" ] || continue
  b=$(basename "$p" .pack)
  cp "$p" "$STAGE/CMDS/$b"
  bytes=$(wc -c < "$STAGE/CMDS/$b" | tr -d ' ')
  echo "$b $bytes" >> "$MANIFEST"
done
# Stage the shippable text sources (assembly + BASIC09) as CR-only. Glob by
# known text extensions, not a bare `*` -- SRC/ also holds the packed .pack
# binaries staged above, and CR-converting one of those would corrupt it.
for s in "$TESTSRC"/*.a "$TESTSRC"/*.i "$TESTSRC"/*.bas; do
  [ -e "$s" ] || continue
  tr '\n' '\r' < "$s" > "$STAGE/SRC/$(basename "$s")"   # OS-9 text is CR-only
done

# --- stage the top-level runner text files, CR-only ---
for t in "$TEXTSRC"/*; do
  tr '\n' '\r' < "$t" > "$STAGE/$(basename "$t")"
done

# --- the os9exec build procedure, CR-only (LF would make it ONE line) ---
{
  echo "mount -k=360k h7"
  echo "login $ACCT"
  for d in CMDS SRC DOCS SCRATCH RESULTS REBUILT; do echo "makdir /h7/$d"; done
  # t05fna's fixture (Step 3): owned by this build account, public read
  # cleared -- -n same reasoning as every other copy below (a fresh FD,
  # not the host mount's synthesized 0.0 ownership); -npr is the 68k
  # spelling that CLEARS public read (6809's own attr inverts this, see
  # references/6809/utility-usage.md). t05fna assumes the account that
  # runs the suite is neither $ACCT nor user ID 0 -- see DOCS/claims.md.
  echo "copy -n /h1/SCRATCH/denied /h7/SCRATCH/denied"
  echo "attr /h7/SCRATCH/denied -npr"
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
    # 68k spelling: -e SETS, -ne clears. Public write is here too, unlike a
    # plain shipped module: `rebuild` (task-4-report.md) replaces this exact
    # file as whatever account the recipient is, not our build account, and
    # OS-9 lets only the owner (or super) overwrite a file otherwise -- see
    # task-4-report.md for the live proof this fails without it.
    echo "attr /h7/CMDS/$b -e -pe -pw -pr"
  done
  for t in "$TEXTSRC"/*; do
    b=$(basename "$t")
    echo "copy -n /h1/$b /h7/$b"          # same -n reasoning as CMDS above
    echo "attr /h7/$b -pr"                # public read: readable by anyone
  done
  # SRC/ itself: Task 3 staged it host-side only ($STAGE/SRC) and left the
  # on-image copy for this task (see task-3-report.md). `rebuild` reads
  # these files on the RECIPIENT's own disk, as whatever account he is --
  # not our build account -- so they need public read same as everything
  # else, not just to exist.
  for s in "$STAGE"/SRC/*; do
    b=$(basename "$s")
    echo "copy -n /h1/SRC/$b /h7/SRC/$b"  # same -n reasoning as CMDS above
    echo "attr /h7/SRC/$b -pr"
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
