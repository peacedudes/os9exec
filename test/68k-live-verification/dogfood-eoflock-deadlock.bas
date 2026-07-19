PROCEDURE eofdlk
! Waiting for yourself. One process, a writer path and a reader path on one
! file: the reader catches up and the file is not finished, because a writer
! still has it open -- but that writer is this same process, and it cannot
! write anything more while it is the one doing the waiting. Sleeping here
! would hang, and hang silently.
!
! So the rule is not "wait whenever a writer is open", it is "wait for
! another process". A conflict with your own process is refused instead:
! E_DEADLK (254). That makes this testable at all -- the alternative is a
! test that hangs, which is the worst kind to own.
!
! Expect "OK deadlock refused err=254". A hang means the writer's identity
! is not being checked; 211 means the file was declared finished while a
! writer was still open, which is the bug this whole mechanism exists to fix.
! Note ERR is captured into <e> as the handler's first act: reading it later,
! after ON ERROR GOTO 0, reports 0 and makes a passing test look confused.
DIM w, r: BYTE
DIM e: INTEGER
DIM line: STRING[20]
ON ERROR GOTO 800
CREATE #w, "dlk.dat": WRITE
OPEN #r, "dlk.dat": READ
WRITE #w, "ONLY"
READ #r, line
PRINT "eofdlk: read="; line
READ #r, line
PRINT "eofdlk: FAIL a second record appeared ="; line
GOTO 900
800 e = ERR
ON ERROR GOTO 0
IF e = 254 THEN 850
PRINT "eofdlk: FAIL err="; e
GOTO 900
850 PRINT "eofdlk: OK deadlock refused err="; e
900 END
