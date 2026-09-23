#!/bin/sh
# Mutation-fuzz the RBF image parser.
#
# os9exec reads a disk image that it did not write: the sector-0 identification
# block, the allocation bitmap, directory sectors and file descriptors are all
# attacker-controlled data as far as the emulator is concerned. This feeds it
# deliberately corrupt ones and watches for the emulator dying rather than
# rejecting them.
#
# Why this is possible now: it used to be listed as BLOCKED in ROADMAP.md for
# two reasons, both gone. Mounting an out-of-tree image no longer crashes (it is
# refused with E_MNF, which is the correct confinement behaviour), and `mount -k`
# gives us an in-tree seed image to mutate, which is what "h1 is a symlink
# outside the repo" was blocking.
#
# Run against an ASan build or this proves very little -- a corrupt image that
# reads one sector off the end of a buffer will usually LOOK fine otherwise:
#   make -B CC="cc -fsanitize=address -fsanitize-ignorelist=tools/ubsan-ignore.txt -g"
#   tools/fuzz-rbf-image.sh 200
#
# Usage: tools/fuzz-rbf-image.sh [iterations]   (default 100)
# Exits 1 when there is a finding, so a caller can tell.
#
# Everything happens in a directory of this run's own: the image is /hf, which
# `mount` finds in the emulator's working directory, so two runs sharing one
# directory (or the repo root, as this once did) mutated each other's image.
set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
emu=$repo/os9exec
iters=${1:-100}
work=$(mktemp -d "${TMPDIR:-/tmp}/os9fuzz.XXXXXX") || exit 1
seed=$work/seed.img
img=$work/hf
crashes=$work/crashes
mkdir -p "$crashes"
cd "$work" || exit 1

# A real, freshly formatted image to mutate. mount -k writes it relative to cwd.
printf 'mount -k=500K /hf\n\033\n' | OS9DISK="${OS9DISK:-}" "$emu" -r shell >/dev/null 2>&1
[ -s "$img" ] || { echo "could not create a seed image"; exit 1; }
cp "$img" "$seed"
echo "seed: $(wc -c <"$seed") bytes"

found=0
i=0
while [ "$i" -lt "$iters" ]; do
    i=$((i+1))

    # Corrupt a handful of bytes, biased towards the metadata at the front:
    # sector 0 (identification), the allocation bitmap and the root directory
    # all live there, and that is the part the parser trusts most.
    python3 - "$seed" "$img" <<'PY'
import random, sys
data = bytearray(open(sys.argv[1],'rb').read())
meta  = min(len(data), 8*1024)
for _ in range(random.randint(1,12)):
    pos = random.randrange(meta) if random.random() < 0.8 else random.randrange(len(data))
    data[pos] = random.randrange(256)
open(sys.argv[2],'wb').write(data)
PY

    # A corrupt image must be REJECTED, not crash the emulator and not hang.
    out=$(printf 'mount /hf\ndir /hf\nfree /hf\ndcheck /hf\n\033\n' \
          | OS9DISK="${OS9DISK:-}" "$emu" -r shell 2>&1)
    rc=$?

    # Signals (>=128) and any sanitizer report are real findings. An OS-9 level
    # "Error #" is the CORRECT answer to a corrupt image, so it is not.
    if [ "$rc" -ge 128 ] || printf '%s' "$out" | grep -q "Sanitizer"; then
        found=$((found+1))
        cp "$img" "$crashes/crash-$i.img"
        printf '%s\n' "$out" > "$crashes/crash-$i.log"
        echo "  [$i] FINDING rc=$rc -- image saved to $crashes/crash-$i.img"
    fi
done

# ---------------------------------------------------------------------------
# Targeted structural probes, with a PROPERTY oracle rather than a crash one.
#
# The random phase above watches for the emulator dying. That oracle is weak
# here for the same reason tools/fuzz-module.sh documents: os9exec installs its
# own SIGSEGV/SIGBUS handler that turns an unmapped read into a guest bus error
# and exits 0, so rc never reaches 128. Worse, some corruption is not a crash
# at all -- it is arithmetic on numbers the media cannot support.
#
# So these cases assert a PROPERTY: an identification sector that is
# structurally impossible must be REFUSED, and a refused image cannot list a
# directory. Offsets are the OS-9 Technical Manual's ("Disk File
# Organization"): DD_TOT $00, DD_MAP $04, DD_BIT $06, DD_DIR $08, DD_LSNSize
# $68.
#
# This phase found a real bug on 2026-09-02: DD_MAP was unvalidated, so a map
# size of $FFFF on a 504K disk had `free` report "128 Mb of 504 Kb (26003%)"
# and scanned directory and file sectors as allocation bits, while DD_BIT=0
# reached a host divide -- silently 0 on arm64, SIGFPE on x86.
echo "targeted structural probes:"

probe() {   # $1=label  $2=offset  $3=size  $4=value  $5=must-list (yes/no)
    python3 - "$seed" "$img" "$2" "$3" "$4" <<'PYEOF'
import sys
d = bytearray(open(sys.argv[1],'rb').read())
off, size, val = int(sys.argv[3],0), int(sys.argv[4]), int(sys.argv[5],0)
if size: d[off:off+size] = val.to_bytes(size,'big')
open(sys.argv[2],'wb').write(d)
PYEOF
    out=$(printf 'mount /hf\ndir /hf\nfree /hf\n\033\n' \
          | OS9DISK="${OS9DISK:-}" "$emu" -r shell 2>&1)
    rc=$?
    listed=no
    printf '%s' "$out" | grep -q "Directory of" && listed=yes
    if [ "$listed" != "$5" ]; then
        found=$((found+1))
        printf '%s\n' "$out" > "$crashes/probe-$1.log"
        echo "  FINDING $1: listed=$listed, expected $5 -- log in $crashes/probe-$1.log"
    else
        echo "  ok  $1 (listed=$listed)"
    fi
    if [ "$rc" -ge 128 ]; then
        found=$((found+1))
        echo "  FINDING $1: died rc=$rc"
    fi
}

# The CONTROL comes first and must LIST. It proves the oracle can tell the
# two outcomes apart -- without it every probe below "passes" on a build that
# refuses everything, or one whose shell is broken, and this whole phase
# becomes unable to fail.
probe control      0x00 0 0          yes
probe dd_dir_wild  0x08 3 0xFFFFFF   no
probe dd_map_wild  0x04 2 0xFFFF     no
probe dd_bit_zero  0x06 2 0x0000     no
probe dd_tot_wild  0x00 3 0xFFFFFF   no
probe dd_lsnsz_bad 0x68 2 0xFFFF     no

echo "done: $iters images, $found findings"
if [ "$found" -eq 0 ]; then
    cd / && rm -rf "$work"
    exit 0
fi
printf '%s\n' "kept in $work; reproduce with: cd $work && cp crashes/crash-N.img hf && printf 'mount /hf\\ndir /hf\\n\\033\\n' | $emu -r shell"
exit 1
