#!/usr/bin/env bash
# build-image.sh -- build the self-contained CONF68K disk.
#
# The output is ONE RBF disk image carrying the whole conformance suite: the
# prebuilt test modules, the runall procedure, the readme, and the claim
# documentation. Mount it on any OS-9/68000 system, `chd` to it, and run. It
# needs no shell utilities beyond the ones OS-9 itself ships, no SDK, no C
# library and no development tools -- every module on it is hand-written 68000
# assembly that calls the kernel directly.
#
# This mirrors tools/selfhost6809/build-image.sh, which does the same job for
# the 6809 suite, deliberately: the two suites should be handed to somebody in
# the same shape.
#
#   tools/selfhost68k/build-image.sh            build build/selfhost68k/conf68k.dsk
#   tools/selfhost68k/build-image.sh --rebuild  reassemble the modules first
#
# WHY THE EMULATOR BUILDS ITS OWN DISK: os9exec can create an RBF image
# (`mount -k`) and populate it (`imakdir`, `icopy`), so the image is built by
# the same RBF code that will later read it. A host-side image writer would be
# a second implementation of the format to keep in step, and the first thing it
# would hide is a bug in the real one.
set -uo pipefail

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SUITE="$REPO/test/68k-conformance"
OUT="$REPO/build/selfhost68k"
IMG="$OUT/conf68k.dsk"
MANIFEST="$OUT/manifest.txt"
EXE="$REPO/os9exec"
TIMEOUT=$(command -v gtimeout || command -v timeout)

[ -n "$TIMEOUT" ] || { echo "need gtimeout or timeout on PATH" >&2; exit 2; }
[ -x "$EXE" ]     || { echo "no os9exec at $EXE -- run make first" >&2; exit 2; }

if [ "${1:-}" = "--rebuild" ]; then
    echo "reassembling modules from SRC/ ..."
    "$REPO/tools/conformance.sh" 68k --build >/dev/null 2>&1 \
      || { echo "rebuild failed -- run tools/conformance.sh 68k --build to see why" >&2; exit 1; }
fi

rm -rf "$OUT"; mkdir -p "$OUT"

# Size the image from what actually goes on it, rounded up generously. t07
# writes a ~70K file into SCRATCH while it runs, and an image with no room for
# that reports ERROR rather than a verdict -- so the headroom is part of the
# suite working, not padding.
payload=$(du -sk "$SUITE/CMDS" 2>/dev/null | awk '{print $1}')
size=$(( payload + 400 ))
[ "$size" -lt 800 ] && size=800

echo "building $IMG (${size}k) ..."
# Created as "h7" and renamed. `mount -k` requires a DEVICE name (h0..hz) and
# says so plainly if given anything else -- it is the emulator's device table
# being addressed, not a filename. The finished artifact wants a name that
# means something to whoever receives it, hence the rename.
mkerr=$( cd "$OUT" && $TIMEOUT 60 "$EXE" -r mount -k=${size}k h7 2>&1 | tr -d '\r' )
if [ ! -f "$OUT/h7" ]; then
    echo "  mount -k did not produce the image:" >&2
    echo "$mkerr" | sed 's/^/    /' >&2
    exit 1
fi
mv "$OUT/h7" "$IMG"

for d in CMDS SCRATCH RESULTS DOCS; do
    $TIMEOUT 60 env OS9H7="$IMG" "$EXE" -r imakdir "/h7/$d" >/dev/null 2>&1
done

: > "$MANIFEST"
copy_in() {   # copy_in <host-relative-path-under-suite> <path-on-image>
    local src="$SUITE/$1" dst="$2"
    [ -f "$src" ] || return 0
    $TIMEOUT 60 env OS9H7="$IMG" OS9H8="$SUITE" "$EXE" \
        -r icopy "/h8/$1" "$dst" >/dev/null 2>&1
    printf '%s %s\n' "$dst" "$(wc -c < "$src" | tr -d ' ')" >> "$MANIFEST"
}

for m in "$SUITE"/CMDS/*; do copy_in "CMDS/$(basename "$m")" "/h7/CMDS/$(basename "$m")"; done
copy_in "runall" "/h7/runall"
copy_in "readme" "/h7/readme"
for d in "$SUITE"/DOCS/*; do
    b=$(basename "$d"); [ -f "$d" ] || continue
    copy_in "DOCS/$b" "/h7/DOCS/$b"
done

n=$(wc -l < "$MANIFEST" | tr -d ' ')
echo "  $n files written, manifest at $MANIFEST"
echo "  image: $IMG ($(du -h "$IMG" | awk '{print $1}'))"
echo
echo "run it here:      tools/selfhost68k/run-image.sh"
echo "verify contents:  tools/selfhost68k/verify-image.sh"
echo "on real OS-9:     put the image on a disk device, chd to it, then: runall"
