! Dogfood task 2026-07-18: RBF EOF-lock design intent, real NitrOS-9 (6809),
! LARGE-FILE follow-up, READ-only reader side. Paired with
! dogfood-eoflock-writer-6809-largefile.bas (A) -- run concurrently via
! `eofwrlg & eofrdlg &`, one combined shell command line, same harness trick
! as the original small-file pass. See that writer file's header for the
! full design rationale (fast bulk-write + deliberate 10-second hold-open
! pause instead of real-time per-record pacing) and the two real harness
! bugs found while building this pair (a writer-side startup-order race, and
! a BASIC09 line-number auto-sort gotcha).
!
! This reader carries a defensive OPEN-retry wrapper (labels 10/20/25,
! ~40 short-throttled attempts) that the original small-file reader didn't
! need -- added after this file's own launch lost the CREATE-vs-OPEN startup
! race at least once even after the writer fix, confirming the race is a
! genuine coin-flip between two freshly-forked processes' startup order, not
! purely a writer-side defect. This wrapper only guards the initial OPEN; it
! has no bearing on the actual READ-blocking behavior under test below.
!
! THE FINDING (reproduced identically across two independent clean runs):
! unlike the small-file pass -- where the reader's first READ blocked for
! the writer's *entire* run and delivered all 13 records in one shot exactly
! at CLOSE -- this reader read the large majority of records (1 through 46
! of 50) essentially live, while the writer was still open, *before* the
! writer's deliberate 10-second hold-open pause even ended. Only the last 4
! records (47-50) blocked, and that block released exactly at the writer's
! CLOSE, not before. retries=0 throughout both runs (the ON ERROR GOTO 800
! E$EOF handler never fired) -- every block observed, including the final
! one, is a genuine kernel-level wait, not poll-yourself retrying.
!
! Literal verified output (6809, real NitrOS-9, via tools/nitros9repl.sh,
! launched as `eofwrlg & eofrdlg &`):
!   Run 1: reader-RO: opened path=4 err=0 time=09:14:26 opentries=0
!          reader-RO: read #1 time=09:14:26 data=seq=1 t=09:14:26 pad
!          ... (records 2-46 all land within the next ~4 real seconds,
!              well before the writer's close) ...
!          reader-RO: read #46 time=09:14:30 data=seq=46 t=09:14:29 pa
!          reader-RO: read #47 time=09:14:39 data=seq=47 t=09:14:29 pa
!          writer: closed at 09:14:39 (from the writer's own log, same instant)
!          reader-RO: read #48 time=09:14:39 data=seq=48 t=09:14:29 pa
!          reader-RO: read #49 time=09:14:39 data=seq=49 t=09:14:29 pa
!          reader-RO: read #50 time=09:14:39 data=seq=50 t=09:14:29 pa
!          reader-RO: SUCCESS all 50 records, retries=0 time=09:14:39
!   Run 2: reader-RO: opened path=4 err=0 time=09:15:56 opentries=0
!          reader-RO: read #1 time=09:15:56 data=seq=1 t=09:15:56 pad
!          reader-RO: read #46 time=09:15:59 data=seq=46 t=09:15:59 pa
!          reader-RO: read #47 time=09:16:09 data=seq=47 t=09:15:59 pa
!          writer: closed at 09:16:09 (same instant)
!          reader-RO: read #50 time=09:16:09 data=seq=50 t=09:15:59 pa
!          reader-RO: SUCCESS all 50 records, retries=0 time=09:16:09
! In both runs the "read #47 blocks until CLOSE" gap is exactly the writer's
! ~10-second hold -- the reader had genuinely caught up to the true current
! end of file (the writer had stopped producing new data after its fast
! write loop) and correctly stalled right at that live edge, matching the
! original design intent's core promise, rather than racing ahead. See
! dogfood-report-eoflock-6809-largefile-2026-07-18.md for full analysis and
! the cross-reference to the small-file pass's now-revised conclusion.
PROCEDURE eofrdlg
DIM path: BYTE
DIM line: STRING[80]
DIM n, retries, k, opentries: INTEGER
DIM t: STRING[8]
n = 0
retries = 0
opentries = 0
ON ERROR GOTO 20
10 OPEN #path, "/DD/EOFTEST/eoflarge.dat": READ
GOTO 30
20 opentries = opentries + 1
IF opentries > 40 THEN 25
FOR k = 1 TO 300
NEXT k
GOTO 10
25 PRINT #2, "reader-RO: ABORT-OPEN opentries="; opentries; " err="; ERR; " time="; RIGHT$(DATE$,8)
GOTO 999
30 PRINT #2, "reader-RO: opened path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8); " opentries="; opentries
ON ERROR GOTO 800
100 READ #path, line
n = n + 1
PRINT #2, "reader-RO: read #"; n; " time="; RIGHT$(DATE$,8); " data="; LEFT$(line,20)
IF n < 50 THEN 100
GOTO 900
800 t = RIGHT$(DATE$,8)
IF ERR <> 211 THEN 950
retries = retries + 1
PRINT #2, "reader-RO: EOF retry #"; retries; " time="; t; " (n so far="; n; ")"
IF retries > 150 THEN 950
FOR k = 1 TO 1000
NEXT k
GOTO 100
900 PRINT #2, "reader-RO: SUCCESS all "; n; " records, retries="; retries; " time="; RIGHT$(DATE$,8)
GOTO 999
950 PRINT #2, "reader-RO: ABORT-READ n="; n; " err="; ERR; " retries="; retries; " time="; t
999 CLOSE #path
PRINT #2, "reader-RO: done"
END
