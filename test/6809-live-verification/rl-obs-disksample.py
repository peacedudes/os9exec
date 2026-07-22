#!/usr/bin/env python3
"""Host-side .ide record sampler -- zero guest perturbation.

Polls the 10 bytes of rl.dat's record inside a (private!) .ide image and
logs every distinct state with a host timestamp; each state self-labels in
guest time via its stamps (v=rec(1), w=rec(2), j=rec(3) from rl-race3o).
Every change seen here is a physical flush of the data sector reaching the
host file -- during the 2026-07-21 lost-update runs there were exactly
three (rlinit's close and the two racers' closes): the race is all-RAM.

The record offsets are PER-IMAGE.  Find them by content after any run:
search the image for the record's known final bytes (five big-endian
INTEGERs, e.g. counter=0x0160, id=2, done=2 -> pattern
'01 60 .. .. .. .. 00 02 00 02'); the live copy is the 512-aligned hit.

Usage: rl-obs-disksample.py <image.ide> <offset> [offset...] > flushlog.txt
"""
import sys, time, struct

path = sys.argv[1]
offsets = [int(x, 0) for x in sys.argv[2:]]
last = {}
f = open(path, 'rb', buffering=0)
t0 = time.time()
while True:
    for off in offsets:
        f.seek(off)
        b = f.read(10)
        if last.get(off) != b:
            last[off] = b
            v, w, j, idc, done = struct.unpack('>5h', b)
            print(f"{time.time()-t0:9.3f} off={off} v={v} w={w} j={j} id={idc} done={done}", flush=True)
    time.sleep(0.03)
