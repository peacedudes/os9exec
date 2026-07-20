#!/bin/sh
# Property-fuzz the OS-9 MODULE parser (F$Load path).
#
# os9exec loads a memory module out of a file it did not write: sync word,
# header parity, CRC, total size and the 32-bit name offset _mname are all
# attacker-controlled.  _mname is the dangerous one: Mod_Name() turns it into a
# raw HOST pointer, (char*)module + _mname, that nullterm() then walks -- so an
# out-of-range offset is an out-of-bounds host read that sync/parity/CRC do
# nothing to stop.
#
# WHY THE ORACLE IS "was it rejected", not "did it crash".  A crash oracle (the
# RBF fuzzer's model) is structurally blind to this bug in os9exec, for two
# reasons found the hard way:
#   * get_mem() sub-allocates every module from one big pooled block, so ASan
#     sees the whole pool as a single valid object and never sees an
#     intra-pool over-read -- a wild _mname just reads an adjacent module.
#   * os9exec installs its own SIGSEGV/SIGBUS handler that converts a truly
#     unmapped read into a *guest* bus error and exits 0, so rc never goes >=128.
# So instead this fuzzer forges modules whose _mname is DELIBERATELY out of
# range (or in range but unterminated) -- every one of which the loader MUST
# refuse -- and a module that LOADS anyway is the finding.  That oracle goes red
# on the pre-fix binary (the bad module loads with a garbage name lifted from a
# neighbouring module) and green on the fixed one.  Signals / sanitizer reports
# are still caught as a secondary net.
#
# sync/parity/CRC are all forgeable, so each mutant is repaired to pass them and
# reach the offset check.  The CRC is affine over GF(2); its trailing three
# bytes are SOLVED, not brute-forced.
#
# Best run against an ASan build (secondary net); the primary oracle needs no
# instrumentation:
#   make -B CC="cc -fsanitize=address -fsanitize-ignorelist=tools/ubsan-ignore.txt -g"
#   tools/fuzz-module.sh 300
#
# Usage: tools/fuzz-module.sh [iterations]   (default 100)
set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
cd "$repo" || exit 1
iters=${1:-100}
work=${TMPDIR:-/tmp}/os9modfuzz
mdir=$work/mdir
crashes=$work/crashes
seed=$work/seed

rm -rf "$work"; mkdir -p "$mdir" "$crashes"

cp "$repo/h0/CMDS/free" "$seed" || { echo "no seed module"; exit 1; }
echo "seed: $(wc -c <"$seed") bytes (h0/CMDS/free)"

found=0
i=0
while [ "$i" -lt "$iters" ]; do
    i=$((i+1))

    # Forge a CRC-valid module with a _mname the loader must reject, then repair
    # parity + CRC.  Prints the offset it chose so a finding is self-describing.
    off=$(python3 - "$seed" "$mdir/fuzzmod" <<'PY'
import random, struct, sys

TARGET = 0xFF800FE3

def calc_crc(d, size, accum=0xFFFFFFFF):
    for k in range(size):
        accum &= 0x00FFFFFF
        b = (d[k] << 16) ^ accum
        accum = (accum << 8) & 0xFFFFFFFF
        b >>= 16
        accum = (accum ^ (b << 1) ^ (b << 6)) & 0xFFFFFFFF
        if bin(b & 0xFF).count('1') & 1:
            accum ^= 0x00800021
    return (accum | 0xFF000000) & 0xFFFFFFFF

def repair_parity(d):
    p = 0
    for w in range(23):                 # words 0..22, skip parity word $02E
        p ^= struct.unpack_from('>H', d, w * 2)[0]
    struct.pack_into('>H', d, 0x2E, (~p) & 0xFFFF)

def repair_crc(d, size):
    base = size - 3
    for k in range(3):
        d[base + k] = 0
    r0 = calc_crc(d, size) & 0xFFFFFF
    piv = {}
    for bit in range(24):
        byte, bo = base + bit // 8, bit % 8
        d[byte] ^= 1 << (7 - bo)
        col = (calc_crc(d, size) & 0xFFFFFF) ^ r0
        d[byte] ^= 1 << (7 - bo)
        v, t = col, 1 << bit
        while v:
            hb = v.bit_length() - 1
            if hb in piv:
                pv, pt = piv[hb]; v ^= pv; t ^= pt
            else:
                piv[hb] = (v, t); break
    v, t = (TARGET & 0xFFFFFF) ^ r0, 0
    while v:
        hb = v.bit_length() - 1
        if hb not in piv:
            return False
        pv, pt = piv[hb]; v ^= pv; t ^= pt
    for bit in range(24):
        if t & (1 << bit):
            byte, bo = base + bit // 8, bit % 8
            d[byte] ^= 1 << (7 - bo)
    return calc_crc(d, size) == TARGET

d = bytearray(open(sys.argv[1], 'rb').read())
size = struct.unpack_from('>I', d, 4)[0]
d = d[:size]

# A _mname the loader must refuse: 0 (no name), past the module, or wild.
choices = [0, size, size + random.randint(1, 1 << 20), random.getrandbits(32)]
name = random.choice(choices) & 0xFFFFFFFF
struct.pack_into('>I', d, 0x0C, name)

repair_parity(d)
if repair_crc(d, size):
    open(sys.argv[2], 'wb').write(d)
    print(hex(name))
else:
    open(sys.argv[2], 'wb').write(b'')   # unsolvable; skip (empty file is refused)
    print("skip")
PY
)
    [ "$off" = "skip" ] && continue

    out=$(printf 'load fuzzmod\n\033\n' \
          | OS9MDIR="$mdir" OS9DISK="$repo/h0" ./os9exec -r shell 2>&1)
    rc=$?

    if [ "$rc" -ge 128 ] || printf '%s' "$out" | grep -q "Sanitizer"; then
        reason="host CRASH rc=$rc"
    elif printf '%s' "$out" | grep -q "can't load"; then
        reason=""                         # correctly refused -- the right answer
    else
        reason="ACCEPTED a module with bad _mname=$off"
    fi

    if [ -n "$reason" ]; then
        found=$((found+1))
        cp "$mdir/fuzzmod" "$crashes/crash-$i.mod"
        printf '%s\n' "$out" > "$crashes/crash-$i.log"
        echo "  [$i] FINDING: $reason -- module saved to $crashes/crash-$i.mod"
    fi
done

echo "done: $iters modules, $found findings"
[ "$found" -eq 0 ] || echo "reproduce: cp $crashes/crash-N.mod \$OS9MDIR/fuzzmod && printf 'load fuzzmod\\n\\033\\n' | ./os9exec -r shell"
