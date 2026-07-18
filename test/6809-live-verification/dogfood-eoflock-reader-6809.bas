! Dogfood task 2026-07-18: RBF EOF-lock design intent, real NitrOS-9
! (6809), READ-only reader side. Paired with dogfood-eoflock-writer-6809.bas
! (A) -- run concurrently via `eofwr & eofrdro &`, one combined shell
! command line, so both processes start within the same real second (the
! same harness trick the 68k pass needed, and for the same reason -- a
! second `key` sent while a background job is actively streaming output
! is unreliable). A short FOR/NEXT busy-wait throttles the EOF-retry
! loop, matching the 68k sibling's precaution (an unthrottled retry loop
! there appeared to starve the writer of CPU and required a REPL
! restart) -- as it turned out, this reader's retry path was never
! actually exercised (see below), but the throttle was kept in as
! written since removing it wasn't needed to get a clean result.
!
! Literal verified output (6809, real NitrOS-9, via tools/nitros9repl.sh,
! reproduced identically across two independent clean runs, both
! launched as `eofwr & eofrdro &`):
!   reader-RO: opened path=4 err=0 time=08:04:45
!   reader-RO: read #1 time=08:04:57 data=08:04:45 seq=1
!   reader-RO: read #2 time=08:04:57 data=08:04:46 seq=2
!   ... (all 13 records, in order, correct content, same real second) ...
!   reader-RO: read #13 time=08:04:57 data=08:04:57 seq=13
!   reader-RO: SUCCESS all 13 records, retries=0 time=08:04:57
!   reader-RO: done
!
! THE FINDING: this reader opened at 08:04:45 -- the same real second as
! the writer's very first WRITE -- and its very first READ call did not
! return until 08:04:57, the exact same second the writer wrote its
! final (13th) record and closed. Crucially, retries=0 in both runs:
! the ON ERROR GOTO 800 handler (which increments `retries` and prints
! "EOF retry #N") never fired even once, meaning the first READ call
! itself did not return an E$EOF error at all during those ~12 seconds
! -- it simply did not return, then returned successfully once
! conditions changed. This is genuine kernel-level BLOCKING, not the
! immediate-return/poll-yourself pattern the 68k os9exec pass found.
! Once unblocked, all 13 records (including the one written a full 12
! seconds before the reader's first successful READ) came back
! correctly, in order, in the same real second, with zero further
! retries -- the reader did not lose or skip any data.
!
! This differs from BOTH candidate outcomes the original task set out to
! test, and from os9exec's own bug: it is not "immediate-return polling"
! (68k's behavior), and it is not the idealized "unblocks promptly on
! each write, pipe-like" design intent either (a read of record #1,
! which fully existed and was already flushed to disk two seconds
! before the reader even opened, should not need to wait 12 more seconds
! under the pipe-like design intent -- there is no live edge to stall
! at for already-committed data). What was actually observed looks
! closest to "blocked for the writer's entire remaining lifetime, then
! released everything at once when the writer's path closed" -- see the
! shared report for the two-run reproduction and the reasoning for
! preferring "genuine kernel block" over "reader starved of CPU by the
! writer's busy-wait loop" as the explanation (retries=0 is the load-
! bearing fact either way; not confirmed further at the kernel-source
! level).
PROCEDURE eofrdro
DIM path: BYTE
DIM line: STRING[24]
DIM n, retries, k: INTEGER
DIM t: STRING[8]
n = 0
retries = 0
OPEN #path, "/DD/EOFTEST/eoflock.dat": READ
PRINT #2, "reader-RO: opened path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
ON ERROR GOTO 800
100 READ #path, line
n = n + 1
PRINT #2, "reader-RO: read #"; n; " time="; RIGHT$(DATE$,8); " data="; line
IF n < 13 THEN 100
GOTO 900
800 t = RIGHT$(DATE$,8)
IF ERR <> 211 THEN 950
retries = retries + 1
PRINT #2, "reader-RO: EOF retry #"; retries; " time="; t; " (n so far="; n; ")"
IF retries > 60 THEN 950
FOR k = 1 TO 1000
NEXT k
GOTO 100
900 PRINT #2, "reader-RO: SUCCESS all "; n; " records, retries="; retries; " time="; RIGHT$(DATE$,8)
GOTO 999
950 PRINT #2, "reader-RO: ABORT n="; n; " err="; ERR; " retries="; retries; " time="; t
999 CLOSE #path
PRINT #2, "reader-RO: done"
END
