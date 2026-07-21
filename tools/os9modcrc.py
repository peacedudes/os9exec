#!/usr/bin/env python3
"""Check or repair the CRC and header parity of an OS-9 memory module.

An OS-9 module carries a 24-bit CRC in its last three bytes and a parity
byte at offset 8 covering the eight header bytes before it.  Patching a
module byte invalidates both, and the module is then refused at load
time, so anything that edits a module in place has to put them back.

Usage:
    os9modcrc.py check <module>...      report stored vs computed
    os9modcrc.py fix   <module>...      rewrite in place, report changes
"""

import sys

POLY = 0x800063          # OS-9's CRC-24 generator
CRC_LEN = 3              # trailing CRC bytes, not themselves covered
HDR_PARITY_OFF = 8       # parity byte, covering offsets 0..7
SYNC = b"\x87\xcd"       # module header sync bytes


def module_crc(body):
    """The CRC OS-9 stores for a module body (everything but its own CRC)."""
    crc = 0xFFFFFF
    for byte in body:
        crc ^= byte << 16
        for _ in range(8):
            crc <<= 1
            if crc & 0x1000000:
                crc ^= POLY
            crc &= 0xFFFFFF
    return crc ^ 0xFFFFFF


def header_parity(header):
    """The parity byte OS-9 stores at offset 8, over offsets 0..7."""
    parity = 0xFF
    for byte in header:
        parity ^= byte
    return parity


def read_module(path):
    with open(path, "rb") as f:
        data = bytearray(f.read())
    if len(data) < HDR_PARITY_OFF + CRC_LEN + 1:
        raise ValueError("too short to be a module")
    if bytes(data[:2]) != SYNC:
        raise ValueError("no $87CD module sync -- not a module?")
    return data


def inspect(data):
    """Return (stored_crc, computed_crc, stored_parity, computed_parity)."""
    stored_crc = int.from_bytes(data[-CRC_LEN:], "big")
    computed_crc = module_crc(data[:-CRC_LEN])
    return (stored_crc, computed_crc,
            data[HDR_PARITY_OFF], header_parity(data[:HDR_PARITY_OFF]))


def describe(path, data):
    scrc, ccrc, spar, cpar = inspect(data)
    crc_ok = "ok" if scrc == ccrc else "BAD"
    par_ok = "ok" if spar == cpar else "BAD"
    return (f"{path}: CRC ${scrc:06X} (computed ${ccrc:06X}, {crc_ok})  "
            f"parity ${spar:02X} (computed ${cpar:02X}, {par_ok})")


def main(argv):
    if len(argv) < 3 or argv[1] not in ("check", "fix"):
        sys.exit(__doc__)

    action, paths = argv[1], argv[2:]
    changed = False

    for path in paths:
        try:
            data = read_module(path)
        except (OSError, ValueError) as err:
            print(f"{path}: {err}", file=sys.stderr)
            return 2

        print(describe(path, data))
        if action != "fix":
            continue

        _, ccrc, _, cpar = inspect(data)
        data[HDR_PARITY_OFF] = cpar
        # The parity byte is inside the CRC's coverage, so the CRC has to be
        # computed after it is corrected, not before.
        data[-CRC_LEN:] = module_crc(data[:-CRC_LEN]).to_bytes(CRC_LEN, "big")

        with open(path, "wb") as f:
            f.write(data)
        print(f"{path}: rewritten -> {describe(path, read_module(path))}")
        changed = True

    return 0 if (action == "check" or changed) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
