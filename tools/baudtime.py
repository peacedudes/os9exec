#!/usr/bin/env python3
"""Measure os9exec's baud pacing from the host side.

Turns "that felt slow" into numbers, in a few seconds, without touching the
emulator. Prints lines of a known fixed length through the paced console and
timestamps each one as it arrives, so per-line delivery can be compared
against what the baud rate says it should be.

    tools/baudtime.py 9600 19200 115200

Expected per line = (line length + 1) * 10 / baud seconds -- ten bits per
character, and the +1 for the line terminator.

What it is good for:

  * Confirming the pacing is load-independent. It should be: delivery is
    scheduled on absolute deadlines (consio.c's next_due_us accumulates,
    rather than sleeping for an interval), so a late drain emits everything
    now due in one go instead of letting the delay compound. Run it against
    a busy machine and the totals should barely move.
  * Telling a genuinely slow run apart from one that was killed. A paced
    run that is merely slow still delivers every line; one that stops early
    was interrupted, and the line count says roughly when.
  * Spotting jitter that averages out. A correct total with a large stdev
    means lumpy delivery, which absolute-deadline scheduling can hide.

Findings on hand (2026-07-19, macOS/arm64) for a 64-character line:

    baud     mean      stdev    expected
    9600     68.71ms   0.06     67.7ms
    19200    34.32ms   3.11     33.9ms
    115200    5.68ms   0.00      5.64ms

9600 and 115200 land on theory with almost no spread. 19200 has ~50x the
jitter of its neighbours while still totalling correctly -- unexplained, and
worth a look if that rate ever matters.
"""

import os
import re
import statistics
import subprocess
import sys
import time

LINE_LEN  = 64
LINE_COUNT= 40          # ~10x the 256-byte FIFO, so it is not one burst
FILLER    = "X"
DATAFILE  = "timing.txt"


def build_datafile(diskdir):
    """A file of fixed-length lines, CR-terminated the way OS-9 wants."""
    path= os.path.join( diskdir, DATAFILE )
    body= "\r".join( [FILLER*LINE_LEN]*LINE_COUNT ) + "\r"
    with open( path, "w", newline="" ) as f:
        f.write( body )
    return path


def measure(baud, diskdir, exe):
    env= dict( os.environ )
    env["OS9DISK"]= os.path.abspath( diskdir )

    cmds= ( "chx /dd/CMDS\n"
            "load math cio\n"
           f"tmode baud={baud}\n"
           f"list /dd/{DATAFILE}\n"
            "\x1b\n" )

    proc= subprocess.Popen( [exe, "shell"], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL, env=env )
    proc.stdin.write( cmds.encode() )
    proc.stdin.flush()

    stamps= []
    for raw in proc.stdout:
        if raw.decode( errors="replace" ).strip().startswith( FILLER*4 ):
            stamps.append( time.monotonic() )
    proc.wait( timeout=180 )

    return stamps


def main():
    rates  = sys.argv[1:] or ["9600", "19200", "115200"]
    diskdir= os.environ.get( "OS9DISK", "h0" )
    exe    = os.environ.get( "OS9EXEC", "./os9exec" )

    build_datafile( diskdir )

    for baud in rates:
        stamps= measure( baud, diskdir, exe )
        if len(stamps) < 2:
            print( f"baud={baud}: no data ({len(stamps)} lines) -- did it run?" )
            continue

        gaps    = [ (stamps[i]-stamps[i-1])*1000 for i in range(1,len(stamps)) ]
        expected= (LINE_LEN+1)*10.0/float(baud)*1000

        print( f"baud={baud:>7}  lines={len(stamps):>3}  "
               f"total={(stamps[-1]-stamps[0])*1000:8.1f}ms" )
        print( f"    per line: mean={statistics.mean(gaps):6.2f}ms  "
               f"stdev={statistics.pstdev(gaps):5.2f}  "
               f"min={min(gaps):6.2f}  max={max(gaps):6.2f}  "
               f"(expected {expected:.2f}ms)" )


if __name__ == "__main__":
    main()
