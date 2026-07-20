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
set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
cd "$repo" || exit 1
iters=${1:-100}
work=${TMPDIR:-/tmp}/os9fuzz
seed=$work/seed.img
crashes=$work/crashes

rm -rf "$work"; mkdir -p "$work" "$crashes"

# A real, freshly formatted image to mutate. mount -k writes it relative to cwd.
rm -f "$repo/hf"
printf 'mount -k=500K /hf\n\033\n' | OS9DISK="$repo/h0" ./os9exec -r shell >/dev/null 2>&1
[ -s "$repo/hf" ] || { echo "could not create a seed image"; exit 1; }
cp "$repo/hf" "$seed"
echo "seed: $(wc -c <"$seed") bytes"

found=0
i=0
while [ "$i" -lt "$iters" ]; do
    i=$((i+1))

    # Corrupt a handful of bytes, biased towards the metadata at the front:
    # sector 0 (identification), the allocation bitmap and the root directory
    # all live there, and that is the part the parser trusts most.
    python3 - "$seed" "$repo/hf" <<'PY'
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
          | OS9DISK="$repo/h0" ./os9exec -r shell 2>&1)
    rc=$?

    # Signals (>=128) and any sanitizer report are real findings. An OS-9 level
    # "Error #" is the CORRECT answer to a corrupt image, so it is not.
    if [ "$rc" -ge 128 ] || printf '%s' "$out" | grep -q "Sanitizer"; then
        found=$((found+1))
        cp "$repo/hf" "$crashes/crash-$i.img"
        printf '%s\n' "$out" > "$crashes/crash-$i.log"
        echo "  [$i] FINDING rc=$rc -- image saved to $crashes/crash-$i.img"
    fi
done

rm -f "$repo/hf"
echo "done: $iters images, $found findings"
[ "$found" -eq 0 ] || echo "reproduce with: cp $crashes/crash-N.img $repo/hf && printf 'mount /hf\\ndir /hf\\n\\033\\n' | ./os9exec -r shell"
