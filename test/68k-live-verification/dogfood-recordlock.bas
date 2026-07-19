PROCEDURE reclock
! Does a read actually lock the record it read?
!
! The counter race (dogfood-lostupdate-*.bas) cannot answer that. It only
! shows whether an update was lost, and on a cooperatively scheduled
! emulator a read-modify-write cycle usually completes without any switch
! landing in the middle -- so it passes whether or not a lock exists. To
! tell those apart you have to make a conflict happen and see it refused.
!
! Two paths on one file, both open for update, one process. Path A reads
! the record, which should lock it. Path B then reads the same bytes: a
! conflict. It cannot be waited out, because the only process that could
! release it is the one doing the waiting -- so the answer must be
! E_DEADLK (254), refused rather than hung.
!
! Expect "OK conflicting read refused err=254".
! "FAIL B read it anyway" means no lock was taken: the read handed over a
! record another path was in the middle of updating, which is exactly the
! lost update the mechanism exists to prevent.
DIM a, b: BYTE
DIM e: INTEGER
DIM buf: STRING[8]
ON ERROR GOTO 800
CREATE #a, "rec.dat": WRITE
WRITE #a, "RECORD01"
CLOSE #a
OPEN #a, "rec.dat": UPDATE
OPEN #b, "rec.dat": UPDATE
SEEK #a, 0
READ #a, buf
PRINT "reclock: A read and locked "; buf
SEEK #b, 0
READ #b, buf
PRINT "reclock: FAIL B read it anyway ="; buf
GOTO 900
800 e = ERR
ON ERROR GOTO 0
IF e = 254 THEN 850
PRINT "reclock: FAIL err="; e
GOTO 900
850 PRINT "reclock: OK conflicting read refused err="; e
900 END
