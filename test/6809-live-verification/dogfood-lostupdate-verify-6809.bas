PROCEDURE lostver
! Dogfood 2026-07-19: verification helper for the 6809 lost-update race
! (see lostinit.bas/lostinc.bas). Opens the shared counter file
! READ-only after both racers finish, prints the final count. Expected
! if RBF's automatic record-locking prevented every lost update:
! 2*N = 600 (N=300 per racer).
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
OPEN #path, "/dd/CLAUDE/counter.dat": READ
SEEK #path, 0
GET #path, rec
PRINT #2, "lostver: final count="; rec.count
CLOSE #path
END
