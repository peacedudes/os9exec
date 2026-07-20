PROCEDURE lostver
! Dogfood task 2026-07-18: lost-update race verification helper. Opens
! the shared counter file (see dogfood-lostupdate-init.bas and
! dogfood-lostupdate-incrementer.bas) READ-only after both racer
! instances have finished, and prints the final count. Expected value
! if RBF's automatic record-locking prevented every lost update: 2*N
! where N is the racer's iteration count (300 -> 600).
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh -r,
! account "claude"), run after each of the two independent races
! recorded in dogfood-lostupdate-incrementer.bas's header:
!   B:run lostver
!   lostver: final count=600
!   Ready
! (both times -- see dogfood-report-lostupdate-2026-07-18.md)
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
OPEN #path, "/h1/CLAUDETEST/counter.dat": READ
SEEK #path, 0
GET #path, rec
PRINT #2, "lostver: final count="; rec.count
CLOSE #path
END
