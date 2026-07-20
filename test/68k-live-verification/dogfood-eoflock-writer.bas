PROCEDURE eofwr
! Dogfood task 2026-07-18: RBF EOF-lock design intent (see
! os9-systems-dev's file-managers.md "Record Locking" section). This
! project's owner personally designed the *original* Microware EOF-lock
! mechanism: a slow writer (A) appending to a growing file and a slow
! reader (B) consuming it should be able to coordinate "as if it were a
! pipe" -- B freely reads everything A has flushed and blocks right at
! the current write position until A's next write, rather than mistaking
! "caught up to current EOF" for "writer is done". Writer A here appends
! a timestamped record once per real second, 40 times (~40 real seconds
! -- widened from the 15-20s originally planned once REPL round-trip
! latency between launching A and B turned out to eat several seconds
! per step), to a shared file on a REAL RBF image (/h1/CLAUDETEST --
! deliberately NOT a host-native mount: os9-dev's using-os9exec-repl.md
! notes host-native dirs have no real record-locking underneath, so this
! test only means something run against a genuine RBF disk image).
! Every WRITE lands at the file's current true end, so per
! file-managers.md each one acquires a whole-file EOF lock (released the
! instant the *next* WRITE begins). Paired with
! dogfood-eoflock-reader-readonly.bas (B, opened READ-only) and
! dogfood-eoflock-reader-update.bas (B, opened UPDATE) -- run one of
! those concurrently in a second shell job while this runs; see this
! trio's shared report, dogfood-report-eoflock-2026-07-18.md, for the
! full narrative.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", launched as `eofwr & eofrdro &` at the shell -- a single
! combined command line so both processes start within the same real
! second, avoiding a REPL harness race described in the report):
!   writer: created for WRITE path=3 err=0 time=04:57:23
!   writer: wrote seq=1 time=04:57:24 err=0
!   writer: wrote seq=2 time=04:57:25 err=0
!   ... (one record per second, err=0 every time, no exceptions) ...
!   writer: wrote seq=40 time=04:58:03 err=0
!   writer: closed at 04:58:03 -- done, 40 records written
!
! The writer's own behavior was flawless and boring in every run (4
! independent live runs, ~160 total WRITEs): every single WRITE
! succeeded immediately with ERR=0, landed on the expected one-second
! boundary, and CLOSE completed cleanly. All of this dogfood pass's
! actual findings are on the READER side -- see the two reader files
! and the shared report.
DIM path: BYTE
DIM i: INTEGER
DIM t, lastt: STRING[8]
DIM line: STRING[24]
CREATE #path, "/h1/CLAUDETEST/eoflock.dat": WRITE
PRINT #2, "writer: created for WRITE path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
lastt = RIGHT$(DATE$,8)
i = 0
100 t = RIGHT$(DATE$,8)
IF t = lastt THEN 100
lastt = t
i = i + 1
line = t + " seq=" + STR$(i)
WRITE #path, line
PRINT #2, "writer: wrote seq="; i; " time="; t; " err="; ERR
IF i < 40 THEN 100
CLOSE #path
PRINT #2, "writer: closed at "; RIGHT$(DATE$,8); " -- done, 40 records written"
END
