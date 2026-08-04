#!/usr/bin/env python3
"""Write a blank OS-9 RBF filesystem image, host-side.

A faithful port of os9exec's BuildBlankImage()/RoundSectorCount()
(Source/OS9exec_core/file_rbf.c).

This existed because `mount -k` once buffered the whole image in the 68k arena
and so could not build anything near 32 MB. That is no longer true: it streams
the head and then the zero tail (commit dffd612), and a 125 MB image builds in
well under a second. What this script is still for is building an image with
NO emulator running -- CI, a Makefile, a fresh clone with no system disk.

Verified by byte-for-byte comparison against `mount -k` output, which is a
check between the two builders, not against anything canonical: keep both
sides in step when either changes.
"""
import os, sys, time

BpB           = 8
TOT_POS, TRK_POS, MAP_POS, BIT_POS, DIR_POS = 0x00, 0x03, 0x04, 0x06, 0x08
CRUZ_POS, SECT_POS = 0x60, 0x68
SectsPerTrack = 0x20
DefaultScts   = 8192
KByte         = 1024

# The identification sector template, verbatim from file_rbf.c:209.
RAM_ZERO = bytes([
 0x00,0x20,0x00,0x00,0x04,0x00,0x00,0x01,0x00,0x00,0x05,0x00,0x00,0xbf,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x07,0x03,0x14,0x2d,0x52,
 0x61,0x6d,0x20,0x44,0x69,0x73,0x6b,0x20,0x28,0x43,0x61,0x75,0x74,0x69,0x6f,0x6e,
 0x3a,0x20,0x56,0x6f,0x6c,0x61,0x74,0x69,0x6c,0x65,0xa9,0x00,0x00,0x00,0x00,0x00,
]) + bytes(32) + bytes([
 0x43,0x72,0x75,0x7a,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,
]) + bytes(256 - 0x70)

def _w(b, o, v): b[o:o+2]   = v.to_bytes(2, "big")
def _l(b, o, v): b[o:o+4]   = v.to_bytes(4, "big")

def round_sectors(size_kb, sct=256, clu=1):
    assert clu & (clu-1) == 0, "cluster size must be a power of 2"
    tot   = size_kb * KByte // sct
    tot   = SectsPerTrack * ((tot-1)//SectsPerTrack + 1)   # granulate to tracks
    bits  = (tot-1)//clu + 1
    tot   = clu * bits                                      # granulate to clusters
    if tot == 0: tot = DefaultScts
    return tot, bits

def pick_cluster(size_kb, sct=256, clu=1):
    """Smallest power-of-2 cluster whose allocation map fits in 16 bits.

    The map size is a WORD in the identification sector, so it cannot exceed
    65535 bytes -- one bit per cluster. With 1-sector clusters that caps a
    256-byte-sector disk at about 128 MB. os9exec's own code spots the
    overflow and tells you to raise the cluster size; it does not do it for
    you. Sector size is deliberately left at 256: RAM_zero is a 256-byte
    array and os9exec memcpy's sctSize bytes out of it, so a 512-byte sector
    would read past the end of that array.
    """
    while True:
        _, bits = round_sectors(size_kb, sct, clu)
        if (bits-1)//BpB + 1 <= 0xffff:
            return clu
        clu *= 2

# DD_DAT / DD_NAM, per the OS-9 Technical Manual's "Disk File Organization"
# table. NAME_MAX used to be 0x3a-0x1f+1 == 28, measured off the length of the
# template's own "Ram Disk (Caution: Volatile)" rather than off the field -- so
# a 29..32 character volume name was silently truncated.
DATE_POS           = 0x1a
NAME_POS, NAME_MAX = 0x1f, 32

def set_volume_name(img, name):
    """Stamp the volume name into the identification sector.

    OS-9 strings are high-bit terminated: the last character carries 0x80.
    The stock template reads "Ram Disk (Caution: Volatile)", which is only
    right for the RAM disk `mount -k` was originally written for.
    """
    b = bytearray(img)
    n = name[:NAME_MAX].encode("ascii", "replace")
    if not n:
        return bytes(b)
    b[NAME_POS:NAME_POS+NAME_MAX] = bytes(NAME_MAX)          # clear the old name
    b[NAME_POS:NAME_POS+len(n)-1] = n[:-1]
    b[NAME_POS+len(n)-1] = n[-1] | 0x80                      # terminator
    return bytes(b)

def set_creation_date(img, when=None):
    """Stamp DD_DAT (5 bytes: year-1900, month, day, hour, minute).

    The template carries 02-07-03 20:45, which Microware's own `free` renders
    as "Jul 3, 1902" -- it is a snapshot of whichever disk RAM_zero was dumped
    from, not a zeroed field. os9exec's StampVolume() writes the same five
    bytes from the host clock; this keeps the two builders in agreement.
    """
    b = bytearray(img)
    t = when or time.localtime()
    b[DATE_POS:DATE_POS+5] = bytes([t.tm_year - 1900, t.tm_mon, t.tm_mday,
                                    t.tm_hour, t.tm_min])
    return bytes(b)

def build_blank(size_kb, sct=256, clu=1):
    tot, bits = round_sectors(size_kb, sct, clu)
    mapsize = (bits-1)//BpB + 1
    if mapsize > 0xffff:
        raise SystemExit("cluster size too small for this device")
    allocSize = (bits-1)//(sct*BpB) + 1      # allocation sectors
    allocN    = allocSize * sct * BpB        # allocation bits

    b = bytearray(sct * tot)
    b[0:sct] = RAM_ZERO[:sct]

    f, r   = allocSize + 1, allocSize + 2    # root FD sector, root dir sector
    fN, rN = f*sct, r*sct
    cluRest = (r//clu + 1)*clu - r

    pt = 0x80
    for ii in range(allocN):                 # reserve id+bitmap+fd+dir, and the tail
        if ii <= r//clu or ii >= bits:
            b[sct + ii//BpB] |= pt
        pt >>= 1
        if pt == 0: pt = 0x80

    _l(b, TOT_POS, tot << BpB)
    b[TRK_POS] = SectsPerTrack
    _w(b, MAP_POS, mapsize)
    _w(b, BIT_POS, clu)
    _l(b, DIR_POS, f << BpB)
    _w(b, SECT_POS, sct)

    b[fN] = 0xbf; b[fN+0x08] = 0x01; b[fN+0x0C] = 0x40
    _l(b, fN+0x10, r << BpB); b[fN+0x14] = cluRest

    b[rN] = 0x2e; b[rN+0x01] = 0xae; _w(b, rN+0x1e, f)
    b[rN+0x20] = 0xae;               _w(b, rN+0x3e, f)
    return bytes(b)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        raise SystemExit("usage: mkblank.py <sizeMB> <out.rbf> [sectorsize] [cluster]")
    mb, out = int(sys.argv[1]), sys.argv[2]
    sct = int(sys.argv[3]) if len(sys.argv) > 3 else 256
    clu = int(sys.argv[4]) if len(sys.argv) > 4 else 1
    if len(sys.argv) <= 4:                      # no explicit cluster: choose one
        clu = pick_cluster(mb*1024, sct, clu)
    img = build_blank(mb*1024, sct, clu)
    # Default the volume name to the output file's own name, matching what
    # `mount -k` now does with the device name. RBF_VOLNAME still overrides.
    vol = os.environ.get("RBF_VOLNAME") or os.path.splitext(os.path.basename(out))[0]
    img = set_volume_name(img, vol)
    # DD_DAT resolves to the minute, so a naive `cmp` against `mount -k` output
    # fails whenever the two runs straddle a minute boundary. RBF_EPOCH (a Unix
    # timestamp) pins it, which is what makes that comparison -- and a
    # reproducible release build -- deterministic.
    epoch = os.environ.get("RBF_EPOCH")
    img = set_creation_date(img, time.localtime(int(epoch)) if epoch else None)
    open(out, "wb").write(img)
    print("  wrote %s: %d bytes (%d sectors of %d, cluster %d)" % (out, len(img), len(img)//sct, sct, clu))
