#!/usr/bin/env python3
"""Install an RBF module into the LIVE bootfile of the 68IDE test image.

usage: nitros9-install-rbf.py <rbf.mn> <68IDE.ide>

The live bootfile is NOT the OS9Boot inside the OS-9 partition at container
offset 3077120.  That one is stale: the system boots fine with its RBF's CRC
and 40 bytes of its code zeroed.  Anything installed there simply never runs,
which makes a patched build behave exactly like stock.

Per the image's own README, the container is:
    bytes 0..1023     XRoar header, not seen by the CoCo
    sectors 0..629    68IDE.dsk -- the boot disk -- each 256-byte OS-9 sector
                      stored in a 512-byte slot (the upper 256 bytes are pad)
    sector 630+       the OS-9 partition
The bootstrap file is a raw LSN run on that boot disk, located by DD.BT and
sized by DD.BSZ in its LSN0.
"""
import sys, hashlib

if len(sys.argv) != 3:
    sys.exit(__doc__.strip().splitlines()[2])   # the usage line
RBF_PATH, IMG = sys.argv[1], sys.argv[2]
BASE = 1024          # start of the boot disk inside the container
SLOT = 512           # bytes per stored sector
SECT = 256           # meaningful bytes per sector

d = bytearray(open(IMG, "rb").read())

def rd(n):  return bytes(d[BASE + n*SLOT : BASE + n*SLOT + SECT])
def wr(n, b):
    assert len(b) == SECT
    d[BASE + n*SLOT : BASE + n*SLOT + SECT] = b

s0 = rd(0)
bt  = int.from_bytes(s0[21:24], "big")
bsz = int.from_bytes(s0[24:26], "big")
nsec_old = (bsz + SECT - 1) // SECT
boot = b"".join(rd(bt + i) for i in range(nsec_old))[:bsz]

# Locate RBF by walking the chain; its offset moves if anything earlier changes.
i, found = 0, None
while i < len(boot) - 4:
    if boot[i:i+2] == b"\x87\xcd":
        size = int.from_bytes(boot[i+2:i+4], "big")
        noff = int.from_bytes(boot[i+4:i+6], "big")
        nm, p = "", i + noff
        while p < len(boot):
            c = boot[p]; nm += chr(c & 0x7F)
            if c & 0x80: break
            p += 1
        if nm == "RBF":
            found = (i, size); break
        i += size; continue
    i += 1
if not found:
    sys.exit("RBF not found in live bootfile")
off, old_size = found

new_rbf = open(RBF_PATH, "rb").read()
new_boot = boot[:off] + new_rbf + boot[off+old_size:]
nsec_new = (len(new_boot) + SECT - 1) // SECT
if nsec_new > nsec_old:
    sys.exit(f"needs {nsec_new} sectors vs {nsec_old} allocated; would have to relocate")

padded = new_boot + b"\x00" * (nsec_new * SECT - len(new_boot))
for k in range(nsec_new):
    wr(bt + k, padded[k*SECT:(k+1)*SECT])
d[BASE + 24 : BASE + 26] = len(new_boot).to_bytes(2, "big")   # DD.BSZ
open(IMG, "wb").write(bytes(d))

# Read it straight back out of the image and confirm what actually landed.
d = bytearray(open(IMG, "rb").read())
chk_bsz = int.from_bytes(rd(0)[24:26], "big")
chk = b"".join(rd(bt+i) for i in range((chk_bsz + SECT - 1)//SECT))[:chk_bsz]
assert chk[off:off+len(new_rbf)] == new_rbf, "readback of installed RBF does not match"
print(f"RBF at bootfile offset {off}: {old_size} -> {len(new_rbf)} bytes")
print(f"bootfile {bsz} -> {chk_bsz} bytes, {nsec_old} sectors (unchanged)")
print("readback verified; installed rbf sha:", hashlib.sha1(new_rbf).hexdigest()[:16])
