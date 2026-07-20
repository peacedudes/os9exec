PROCEDURE eofrdro
! Dogfood task 2026-07-18: RBF EOF-lock design intent, READ-only side.
! Paired with dogfood-eoflock-writer.bas (A) -- run concurrently; see
! that file's header for the shared setup and
! dogfood-report-eoflock-2026-07-18.md for the full write-up.
! file-managers.md says "Reads on read-only ... paths never lock
! anything, since those modes can't update records anyway" -- this
! program opens the shared file for READ only (no UPDATE) to find out
! what that actually means for a reader racing a live writer. A short
! busy-wait is inserted between EOF retries (throttled poll, not a tight
! spin) after an earlier unthrottled version generated 100+ retries/sec
! from two accidentally-duplicated reader instances and appeared to
! starve the concurrently-running writer of CPU time in os9exec's
! scheduler -- a real but separate finding from what this file targets,
! noted in the report.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", clean single-instance run, launched as
! `eofwr & eofrdro &` -- one combined shell command line so both
! processes start within the same real second):
!   reader-RO: opened READ-only path=3 err=0 time=04:57:24
!   reader-RO: EOF(211) retry #1 time=04:57:24 (n so far=0)
!   reader-RO: EOF(211) retry #2 time=04:57:25 (n so far=0)
!   reader-RO: EOF(211) retry #3 time=04:57:26 (n so far=0)
!   ... (retries continue in lockstep with the writer's once-per-second
!        WRITEs -- confirmed via the writer's own concurrent trace,
!        which shows "writer: wrote seq=N" advancing normally through
!        seq=40 and "writer: closed at 04:58:03" while this reader was
!        running) ...
!   reader-RO: EOF(211) retry #401 time=04:58:14 (n so far=0)
!   reader-RO: ABORT n=0 err=0 retries=401 time=04:58:14
!   reader-RO: done
!
! THE FINDING: this reader opened essentially at the writer's very
! first WRITE (n so far=0 from retry #1) and NEVER completed a single
! successful READ -- not one record, out of 40 the writer wrote to that
! exact file while this path stayed open, and not even in the 11
! seconds *after* the writer fully closed (04:58:03) and before this
! reader's own retry cap gave up (04:58:14). This is not a one-off: an
! earlier run opened this same reader when the writer already had 34
! records flushed (backlog present, no startup race possible) and still
! saw zero successful reads, all the way through writer close, for 401
! retries. A brand-new OPEN issued *after* the writer closes reads all
! 40 records back-to-back with zero retries (confirmed separately,
! multiple times) -- so the file and its data are completely fine; the
! bug is specific to a READ-only path that was open *while* a writer
! path was concurrently open on the same file. That path never regains
! visibility into the file's contents for its own remaining lifetime,
! independent of how much data existed when it opened or whether the
! writer has since closed. See the report for the full analysis and how
! this compares to the two candidate outcomes the task set out to
! distinguish (neither one matches what was observed).
DIM path: BYTE
DIM line: STRING[24]
DIM n, retries, k: INTEGER
DIM t: STRING[8]
n = 0
retries = 0
OPEN #path, "/h1/CLAUDETEST/eoflock.dat": READ
PRINT #2, "reader-RO: opened READ-only path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
ON ERROR GOTO 800
100 READ #path, line
n = n + 1
PRINT #2, "reader-RO: read #"; n; " time="; RIGHT$(DATE$,8); " data="; line
IF n < 40 THEN 100
GOTO 900
800 t = RIGHT$(DATE$,8)
IF ERR <> 211 THEN 950
retries = retries + 1
PRINT #2, "reader-RO: EOF(211) retry #"; retries; " time="; t; " (n so far="; n; ")"
IF retries > 400 THEN 950
FOR k = 1 TO 8000
NEXT k
GOTO 100
900 PRINT #2, "reader-RO: SUCCESS all "; n; " records read, total retries="; retries; " time="; RIGHT$(DATE$,8)
GOTO 999
950 PRINT #2, "reader-RO: ABORT n="; n; " err="; ERR; " retries="; retries; " time="; t
999 CLOSE #path
PRINT #2, "reader-RO: done"
END
