PROCEDURE recsem
! The two rules that make one lock per path workable, both easy to get wrong:
!
!   1. A seek does not touch the lock. Moving around a file is not letting
!      go of the record you read; only a write (or a close) releases it.
!   2. A path holds at most ONE record. Reading a second one supersedes the
!      first, rather than accumulating -- so a path that reads its way
!      through a file does not gradually lock the whole thing.
!
! Both are checked through a second path in the same process, which conflicts
! deterministically (E_DEADLK, 254) instead of blocking -- no timing, and it
! cannot hang.
!
! NOTE the two records are deliberately 600+ bytes apart. A read locks the
! count it ASKED for (the manual's rule), and BASIC09 asks for 511 bytes
! whatever the target string's size -- so two records closer together than
! that always overlap, and rule 2 could never be seen. That coarseness is a
! known, accepted cost of the requested-extent rule, not a bug.
!
! NOTE also the data file is CR-terminated records, not one long line:
! BASIC09's READ on a text file reads to the terminator, so a single line
! would make both paths lock the same span and rule 2 could never be seen.
!
! Expect all three OK lines. "FAIL seek released it" means a seek dropped the
! lock; "FAIL still holding rec1" means locks accumulate instead of moving.
DIM a, b: BYTE
DIM e: INTEGER
DIM buf: STRING[8]
ON ERROR GOTO 800
OPEN #a, "recsem.dat": UPDATE
OPEN #b, "recsem.dat": UPDATE
SEEK #a, 0
READ #a, buf
PRINT "recsem: A holds rec1 (", buf, ")"
! --- rule 1: a seek must not release it
SEEK #a, 610
e = 0
ON ERROR GOTO 700
SEEK #b, 0
READ #b, buf
PRINT "recsem: FAIL seek released it"
GOTO 900
700 e = ERR
ON ERROR GOTO 800
IF e <> 254 THEN 810
PRINT "recsem: OK   rule 1, seek did not release"
! --- rule 2: reading rec2 supersedes rec1
SEEK #a, 610
READ #a, buf
PRINT "recsem: A moved to rec2 (", buf, ")"
SEEK #b, 0
READ #b, buf
PRINT "recsem: OK   rule 2, rec1 free after A moved on"
! --- and rec2 is the one held now
e = 0
ON ERROR GOTO 750
SEEK #b, 610
READ #b, buf
PRINT "recsem: FAIL rec2 was not locked"
GOTO 900
750 e = ERR
ON ERROR GOTO 800
IF e <> 254 THEN 810
PRINT "recsem: OK   rule 2, rec2 is held now"
GOTO 900
800 e = ERR
810 ON ERROR GOTO 0
PRINT "recsem: FAIL err="; e
900 END
