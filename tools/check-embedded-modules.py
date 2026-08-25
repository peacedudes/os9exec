#!/usr/bin/env python3
"""Verify the 68k modules compiled into os9exec as constant byte arrays.

modstuff.c embeds four real OS-9/68k modules -- OS9exec, init, socket, le0 --
as `const byte X[]` hex arrays, so that F$Link can find them with no disk
present.  That is what makes `mount -k`, `icopy` and `imakdir` usable before
there is any OS-9 system to load a module FROM.

Nothing checked them.  A byte edited by hand, a bad merge, or an array whose
CRC was never recomputed would be found only at run time, by a guest, as a
module that fails to link or misbehaves -- and the arrays are 100+ lines of
hex apiece, so a reviewer cannot see it either.  This checks what the module
format itself promises:

    sync word        $4AFC at offset 0
    _msize           the size field equals the array length
    header parity    one's-complement XOR of the words below $2E
    CRC-24           the trailing three bytes, OS-9's $800063 polynomial

The size CONSTANT (`#define sizeof_X`) is tied to its array by a
_Static_assert in modstuff.c instead, which is cheaper and catches the same
class at compile time.

Usage:
    tools/check-embedded-modules.py            check the four arrays
    tools/check-embedded-modules.py --selftest prove the checks can fail
"""

import re
import sys

SRC = "Source/OS9exec_core/modstuff.c"
MODULES = ("OS9exec_mod", "Init_mod", "Socket_mod", "Le0_mod")
POLY = 0x800063          # OS-9's CRC-24 generator
PARITY_OFF = 0x2E        # header parity word, covering everything below it


def crc24(data):
    crc = 0xFFFFFF
    for b in data:
        crc ^= b << 16
        for _ in range(8):
            crc <<= 1
            if crc & 0x1000000:
                crc ^= POLY
    return (~crc) & 0xFFFFFF


def header_parity(b):
    par = 0
    for i in range(0, PARITY_OFF, 2):
        par ^= int.from_bytes(b[i:i + 2], "big")
    return (~par) & 0xFFFF


def extract(src, name):
    m = re.search(r"const byte " + name + r"\[\][^=]*= \{(.*?)\};", src, re.S)
    if not m:
        return None
    body = re.sub(r"//[^\n]*", "", m.group(1))     # strip the ASCII gutter
    return bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", body))


def check(name, b):
    """Return a list of problems; empty means the module is well formed."""
    bad = []
    if len(b) < PARITY_OFF + 2 + 3:
        return [f"only {len(b)} bytes -- too short to be a module"]
    if b[0:2] != b"\x4a\xfc":
        bad.append(f"sync {b[0]:02X}{b[1]:02X}, expected 4AFC")
    msize = int.from_bytes(b[4:8], "big")
    if msize != len(b):
        bad.append(f"_msize {msize}, array is {len(b)}")
    stored = int.from_bytes(b[PARITY_OFF:PARITY_OFF + 2], "big")
    if stored != header_parity(b):
        bad.append(f"header parity {stored:04X}, computed {header_parity(b):04X}")
    stored = int.from_bytes(b[-3:], "big")
    if stored != crc24(b[:-3]):
        bad.append(f"CRC {stored:06X}, computed {crc24(b[:-3]):06X}")
    return bad


def selftest():
    """Every check above must be able to fail, or it is decoration."""
    src = open(SRC, encoding="utf-8", errors="surrogateescape").read()
    good = extract(src, "Le0_mod")
    if good is None or check("Le0_mod", good):
        print("SELFTEST FAIL: Le0_mod is not clean to begin with")
        return 1

    cases = [("sync", 1, 0xFF), ("_msize", 5, 0xFF),
             ("header parity", PARITY_OFF + 1, 0xFF), ("CRC", len(good) - 1, 0xFF)]
    ok = True
    for label, off, val in cases:
        b = bytearray(good)
        b[off] ^= val
        problems = check("x", bytes(b))
        hit = any(label.split()[0].lower() in p.lower() for p in problems)
        print(f"  flip byte {off:#05x} -> {'detected' if hit else 'MISSED'} ({label})")
        ok = ok and hit
    print("SELFTEST", "OK -- every check can fail" if ok else "FAIL")
    return 0 if ok else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    src = open(SRC, encoding="utf-8", errors="surrogateescape").read()
    rc = 0
    for name in MODULES:
        b = extract(src, name)
        if b is None:
            print(f"{name:12s} NOT FOUND in {SRC}")
            rc = 1
            continue
        problems = check(name, b)
        print(f"{name:12s} {len(b):4d} bytes  " +
              ("ok" if not problems else "; ".join(problems)))
        if problems:
            rc = 1
    return rc


if __name__ == "__main__":
    sys.exit(main())
