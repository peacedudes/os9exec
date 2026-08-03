#!/bin/bash
# Build an RBF disk image from a host directory tree.
#
#   mkimage.sh <source-tree> <output-image> [sizeMB]
#
# TWO SESSIONS, and that is the whole trick:
#
#   1. `mount -k` builds the image IN THE 68k ARENA -- `free` reports the
#      volume as "Ram Disk (Caution: Volatile)". A 27M image therefore leaves
#      almost nothing of the 32M arena, and the very next `copy` dies with
#      E_NORAM. So session 1 creates the image and exits immediately.
#   2. An image that already exists on disk mounts normally and costs no
#      arena, so session 2 does all the makdir/copy work.
#
# The image is created as <startPath>/hz because os9exec only mounts an image
# whose FILENAME is a device name (see ROADMAP.md -- fixed but uncommitted).
# It is moved to <output-image> at the end.
#
# copy -b=4 (a 4K buffer, not the default): the mounted image occupies the 68k
# arena, so a 27M image leaves too little for copy to load csl AND buffer a
# large file -- "No more memory !!!" partway through, on a 164K file.
#
# Attributes are normalised at the end: OS-9 modules (4AFC magic) get
# e+pe+r+pr, everything else r+pr and no execute -- so the bits describe what
# the file IS rather than whatever the host filesystem happened to have.
set -u
R=/Users/rdoggett/mine/os9/XXX/os9exec/os9exec-git_code
SRC=$1; OUT=$2; MB=${3:-0}
DEV=hz
WORK=$R/$DEV

[ -e "$WORK" ] && { echo "refusing: $WORK already exists"; exit 1; }
[ -d "$SRC" ]  || { echo "no such tree: $SRC"; exit 1; }
# Leave real room: games write score files, saves and lock files, flex and
# rayshade want /dd/tmp, and a disk with no free space is a museum piece.
[ "$MB" -eq 0 ] && MB=$(( $(du -sm "$SRC" | cut -f1) * 3 + 32 ))
echo "  $SRC ($(du -sh "$SRC" | cut -f1)) -> ${MB}M image"

# ---- blank image, written host-side
# NOT `mount -k`: that builds the whole image in the 68k arena and so cannot
# create anything near the 32M arena size. mkblank.py is a port of os9exec's
# own BuildBlankImage() and is verified byte-for-byte identical to `mount -k`
# output at 4M, 12M and 28M -- with no size ceiling.
python3 "$(dirname "$0")/mkblank.py" "$MB" "$WORK" >/dev/null || exit 1
[ -e "$WORK" ] || { echo "  FAILED: image not created"; exit 1; }

# ---- populate, in BATCHES of separate os9exec sessions
# os9exec leaks arena memory across process creations: a straight run dies with
# "No more memory !!!" / "can't install csl" after ~487 copies, on a file of a
# few hundred bytes -- the count is what matters, not the size. Each batch is a
# fresh session, so the arena starts clean every time.
LIST=$(mktemp -d)
( cd "$SRC" && find . -mindepth 1 -type d | sed 's|^\./||' | sort ) > "$LIST/dirs"
( cd "$SRC" && find . -type f          | sed 's|^\./||' | sort ) > "$LIST/files"
ndirs=$(wc -l < "$LIST/dirs" | tr -d ' '); nfiles=$(wc -l < "$LIST/files" | tr -d ' ')
echo "  $ndirs dirs, $nfiles files"

run() { ( cd "$R" && gtimeout 1200 env OS9DISK="$R/h0" OS9H6="$SRC" OS9H7="$LIST" \
            ./os9exec -r shell < "$1" 2>&1 ) | tr -d '\000' \
          | grep -aiE "can't|error|no more memory" | head -4; }

c=$(mktemp)
{ echo "chx /dd/CMDS"; sed 's|^|makdir /'"$DEV"'/|' "$LIST/dirs"; printf '\033\n\004\n'; } > "$c"
run "$c"

BATCH=250
i=0
while [ $i -lt $nfiles ]; do
  { echo "chx /dd/CMDS"
    sed -n "$((i+1)),$((i+BATCH))p" "$LIST/files" \
      | while read -r f; do echo "copy -b=4 -r /h6/$f /$DEV/$f"; done
    printf '\033\n\004\n'; } > "$c"
  run "$c"
  i=$((i+BATCH))
  printf '\r    copied %d/%d' "$( [ $i -gt $nfiles ] && echo $nfiles || echo $i )" "$nfiles"
done
echo ""

# attribute lists (CR-terminated: attr -z= reads an OS-9 text file)
while read -r f; do
  if [ "$(head -c 2 "$SRC/$f" 2>/dev/null | od -An -tx1 | tr -d ' \n')" = "4afc" ]
    then printf '/%s/%s\r' "$DEV" "$f" >> "$LIST/mods"
    else printf '/%s/%s\r' "$DEV" "$f" >> "$LIST/data"
  fi
done < "$LIST/files"
{ echo "chx /dd/CMDS"
  [ -s "$LIST/mods" ] && echo "attr -a -z=/h7/mods -e -pe -r -pr -w"
  [ -s "$LIST/data" ] && echo "attr -a -z=/h7/data -r -pr -ne -npe"
  printf '\033\n\004\n'; } > "$c"
run "$c"
rm -f "$c"; rm -rf "$LIST"

mv "$WORK" "$OUT"
echo "  wrote $OUT ($(du -h "$OUT" | cut -f1))"
