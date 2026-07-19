PROCEDURE lockhold
! Holds a record locked across a visible stretch of time, so another process
! trying to touch the same bytes has something to actually wait on.
! A read on an update-mode path takes the lock; the following write releases
! it. Between the two this process deliberately does console I/O, both to
! timestamp what it is doing and to yield the CPU -- without a yield the
! waiter never runs at all and the test proves nothing.
! Paired with dogfood-recordlock-waiter.bas; run as: lockhold & lockwait
DIM p: BYTE
DIM i, j: INTEGER
DIM buf: STRING[8]
OPEN #p, "/h1/CLAUDETEST/rec2.dat": UPDATE
SEEK #p, 0
READ #p, buf
PRINT #2, "hold: locked at ", RIGHT$(DATE$,8)
! The inner loop buys real time; the PRINT is what yields the CPU, since
! without a system call nothing takes it away (see -q). Both are needed:
! time alone would starve the waiter, a yield alone would be instantaneous.
FOR i = 1 TO 12
FOR j = 1 TO 400
NEXT j
PRINT #2, "hold: still holding ", RIGHT$(DATE$,8)
NEXT i
SEEK #p, 0
WRITE #p, "RECORD02"
PRINT #2, "hold: released at ", RIGHT$(DATE$,8)
CLOSE #p
END
