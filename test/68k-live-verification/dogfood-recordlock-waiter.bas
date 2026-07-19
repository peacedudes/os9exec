PROCEDURE lockwait
! Tries to read a record another process holds locked. Should not come back
! until that process writes the record back. If it returns immediately, the
! read handed over a record mid-update -- the lost update the lock exists to
! prevent. Compare its "got it" time against the holder's "released" time.
DIM p: BYTE
DIM buf: STRING[8]
OPEN #p, "/h1/CLAUDETEST/rec2.dat": UPDATE
PRINT #2, "wait: asking at ", RIGHT$(DATE$,8)
SEEK #p, 0
READ #p, buf
PRINT #2, "wait: got it at ", RIGHT$(DATE$,8), " data=", buf
CLOSE #p
END
