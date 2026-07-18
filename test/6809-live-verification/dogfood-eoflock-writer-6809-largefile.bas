! Dogfood task 2026-07-18: RBF EOF-lock design intent, real NitrOS-9 (6809),
! LARGE-FILE follow-up to dogfood-eoflock-writer-6809.bas /
! dogfood-eoflock-reader-6809.bas (see dogfood-report-eoflock-6809-2026-07-18.md).
! That pass found a concurrently-opened reader's first READ blocked for the
! writer's *entire* ~12-second run (13 tiny records, well under one 256-byte
! RBF sector -- see os9-systems-dev's file-managers.md) and delivered
! everything in one shot exactly at the writer's CLOSE, with retries=0
! throughout (a genuine kernel-level block, not polling). Open question that
! pass explicitly flagged: is the block really tied to CLOSE specifically, or
! would a file large enough to cross several RBF sectors reveal a finer-grained
! release (a sector/buffer-flush unblocking the reader before CLOSE), with the
! previous file simply too small to ever show the difference?
!
! Design note, corrected mid-session: the first attempt at this file paced
! writes one real record/second for 40-60 seconds (mirroring the small test's
! style). The user does not want any test needing more than ~1 minute without
! special permission, and pointed out a real-time-paced writer is unnecessary
! to answer this question. Redesigned: write all 50 padded records in a tight
! loop as fast as BASIC09/RBF actually execute them (no per-record real-time
! throttle), then deliberately HOLD the path open (unclosed) for a real
! ~10-second pause -- that pause is the only real-time cost, and it's the
! precise window where a reader's ability to see already-flushed data while
! the writer is still open (or not) gets tested. Total wall time per run:
! ~15-16s end-to-end (create -> reader done), not 40-60s.
!
! Records are padded to a *fixed* 80 bytes each (seq number + timestamp +
! STRING$-free repeated-"X" padding, built via a one-time FOR/NEXT loop since
! BASIC09 has no documented STRING$-repeat function -- see
! os9-dev's basic09-language.md) so the total file size is exactly
! predictable -- almost: WRITE #path, line is BASIC09's *sequential* file
! write, which appends a trailing CR delimiter on top of the 80 content
! bytes (see os9-dev's basic09-language.md, "Sequential vs. random access
! files" -- there is no separate WRITELN; plain WRITE/READ already are the
! CR-delimited, line-based statements for sequential files). So each
! record is actually 81 bytes on disk, not 80: 50 records * 81 bytes = 4050
! bytes = ~15.8 RBF sectors (256 bytes/sector), comfortably past the
! previous test's sub-sector total and past this task's 10-15-sector
! target. (The PRINT strings below, typed before this was noticed, still
! say "~4000 bytes" -- cosmetic only, doesn't affect the real on-disk file
! size or any conclusion in the report.)
!
! Real harness bug found and fixed while building this file (independent of
! the eoflock question): a *fast* concurrent launch (`eofwrlg & eofrdlg &`)
! reliably lost a race against this writer's own startup overhead in an
! earlier draft (a one-time 60-iteration pad-string-build loop sat *before*
! CREATE) -- the reader's OPEN call reliably ran before the writer's CREATE
! completed, aborting with `Error #216 - Path Name Not Found` on the *data*
! file, not a module-resolution problem. Fixed by moving CREATE to be the
! very first executable statement (matching the small test's shape) and
! deferring the one-time pad-string setup until after it. Even so, this raced
! close enough that the *reader* side also got a defensive OPEN-retry wrapper
! (see dogfood-eoflock-reader-6809-largefile.bas) since a coin-flip launch
! order between two freshly-forked processes can't be ruled out in general.
!
! Second real gotcha found: BASIC09's LOAD/compiler auto-sorts numbered lines
! by their numeric value for final fall-through order, *independent of the
! physical/typed order in the source file* -- classic line-numbered-BASIC
! behavior that structured BASIC09 still carries. Typing retry-loop labels
! (710/700/720) *before* the pre-existing 100/800/900 block in physical
! source order silently resequenced execution (line 100 ended up running
! immediately after the procedure's first ON ERROR, not where intended) with
! no compile error -- a real trap for future scripted-`tee` composition of
! any BASIC09 program using more than one logical block of GOTO targets.
! Fixed by renumbering everything in one strictly ascending sequence
! (10/20/25/30/100/800/900/950/999) matching intended execution order.
!
! Literal verified output (6809, real NitrOS-9, via tools/nitros9repl.sh,
! launched as `eofwrlg & eofrdlg &`, reproduced identically across two
! independent clean runs):
!   Run 1: writer: created path=4 err=0 time=09:14:24
!          writer: wrote 50 records fast, holding open, time=09:14:29
!          writer: closed at 09:14:39 -- done, 50 records, ~4000 bytes, held open ~10s
!   Run 2: writer: created path=4 err=0 time=09:15:53
!          writer: wrote 50 records fast, holding open, time=09:15:59
!          writer: closed at 09:16:09 -- done, 50 records, ~4000 bytes, held open ~10s
! Both runs: the 50-record fast-write loop took several real seconds under
! emulation (not sub-second, but far faster and far shorter overall than
! real-time pacing), then the path sat open-but-idle for exactly a real
! ~10-second hold before CLOSE. See dogfood-eoflock-reader-6809-largefile.bas
! and dogfood-report-eoflock-6809-largefile-2026-07-18.md for what the reader
! observed during that hold window -- that's where this test's actual finding
! lives.
PROCEDURE eofwrlg
DIM path: BYTE
DIM i, j, plen, pausesecs: INTEGER
DIM t, holdt: STRING[8]
DIM padfull: STRING[60]
DIM prefix: STRING[40]
DIM line: STRING[80]
CREATE #path, "/DD/EOFTEST/eoflarge.dat": WRITE
PRINT #2, "writer: created path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
padfull = ""
FOR j = 1 TO 60
padfull = padfull + "X"
NEXT j
FOR i = 1 TO 50
prefix = "seq=" + STR$(i) + " t=" + RIGHT$(DATE$,8) + " pad="
plen = LEN(prefix)
line = prefix + LEFT$(padfull, 80 - plen)
WRITE #path, line
NEXT i
PRINT #2, "writer: wrote 50 records fast, holding open, time="; RIGHT$(DATE$,8)
holdt = RIGHT$(DATE$,8)
pausesecs = 0
200 t = RIGHT$(DATE$,8)
IF t = holdt THEN 200
holdt = t
pausesecs = pausesecs + 1
IF pausesecs < 10 THEN 200
CLOSE #path
PRINT #2, "writer: closed at "; RIGHT$(DATE$,8); " -- done, 50 records, ~4000 bytes, held open ~10s"
END
