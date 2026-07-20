PROCEDURE eofrdup
! Dogfood task 2026-07-18: RBF EOF-lock design intent, UPDATE-mode side.
! Paired with dogfood-eoflock-writer.bas (A) -- run concurrently; see
! that file's header for the shared setup and
! dogfood-report-eoflock-2026-07-18.md for the full write-up. Opening
! for UPDATE (not READ-only) means this reader's own reads DO
! participate in RBF's real record-locking machinery, per
! file-managers.md's "Read (or ReadLn) on a path opened for update
! locks the bytes ... for the requested count" -- the question was
! whether that changes the outcome compared to the READ-only sibling
! file. Same throttled-retry trace shape as
! dogfood-eoflock-reader-readonly.bas so the two runs are directly
! comparable.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", clean single-instance run, launched as
! `eofwr & eofrdup &` -- one combined shell command line):
!   reader-UP: opened UPDATE path=3 err=0 time=04:58:42
!   reader-UP: EOF(211) retry #1 time=04:58:43 (n so far=0)
!   reader-UP: EOF(211) retry #2 time=04:58:44 (n so far=0)
!   ... (retries continue in lockstep with the writer's once-per-second
!        WRITEs, confirmed via the writer's own concurrent trace, which
!        shows "writer: wrote seq=N" advancing normally through seq=40
!        and "writer: closed at 04:59:21" while this reader was
!        running) ...
!   reader-UP: EOF(211) retry #401 time=04:59:32 (n so far=0)
!   reader-UP: ABORT n=0 err=0 retries=401 time=04:59:32
!   reader-UP: done
!
! THE FINDING: opening for UPDATE instead of READ-only made NO
! observable difference -- this clean single-instance run shows the
! identical pattern as the READ-only sibling: zero successful reads,
! for the reader's entire lifetime, continuing 11 seconds past the
! writer's close (04:59:21) with no change, until this reader's own
! retry cap gave up. A separate, later, non-concurrent run of this same
! program (writer already closed, no contention) read all 40 records
! back-to-back with zero retries -- confirming UPDATE-mode reads work
! completely normally once there's no concurrently-open writer path;
! the bug (see the READ-only sibling file's header) is specific to
! concurrency, not to either open mode. One earlier, messier run (two
! reader-UP instances accidentally launched at once by a REPL send
! that appeared to silently fail and was retried -- see the report's
! "harness friction" section) showed ONE of the two duplicate instances
! successfully reading up to record #29 before the trace became too
! interleaved to follow reliably; that result did not reproduce in this
! clean single-instance run and is flagged in the report as an
! unconfirmed, likely lock-interaction artifact of having two
! concurrent UPDATE paths rather than a real capability of UPDATE mode
! -- the clean, reproducible result recorded here (and confirmed by a
! second clean single-instance UPDATE run, not separately archived) is
! that UPDATE mode is just as locked out as READ-only.
DIM path: BYTE
DIM line: STRING[24]
DIM n, retries, k: INTEGER
DIM t: STRING[8]
n = 0
retries = 0
OPEN #path, "/h1/CLAUDETEST/eoflock.dat": UPDATE
PRINT #2, "reader-UP: opened UPDATE path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
ON ERROR GOTO 800
100 READ #path, line
n = n + 1
PRINT #2, "reader-UP: read #"; n; " time="; RIGHT$(DATE$,8); " data="; line
IF n < 40 THEN 100
GOTO 900
800 t = RIGHT$(DATE$,8)
IF ERR <> 211 THEN 950
retries = retries + 1
PRINT #2, "reader-UP: EOF(211) retry #"; retries; " time="; t; " (n so far="; n; ")"
IF retries > 400 THEN 950
FOR k = 1 TO 8000
NEXT k
GOTO 100
900 PRINT #2, "reader-UP: SUCCESS all "; n; " records read, total retries="; retries; " time="; RIGHT$(DATE$,8)
GOTO 999
950 PRINT #2, "reader-UP: ABORT n="; n; " err="; ERR; " retries="; retries; " time="; t
999 CLOSE #path
PRINT #2, "reader-UP: done"
END
