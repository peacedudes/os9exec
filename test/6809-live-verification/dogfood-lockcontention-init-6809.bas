PROCEDURE lockinit
! Setup helper for lockholder.bas/lockwaiter.bas -- creates the shared
! record file fresh. Same 10-byte record shape as the lost-update test
! (count + 4 INTEGER filler fields), a fresh file each run.
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
CREATE #path, "/dd/CLAUDE/locktest.dat": UPDATE
rec.count = 0
SEEK #path, 0
PUT #path, rec
CLOSE #path
PRINT #2, "lockinit: created locktest.dat, count=0"
END
