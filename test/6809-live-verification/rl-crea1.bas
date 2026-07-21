PROCEDURE rlcreaw
(* Write-only CREATOR, one record roughly every second -- the Creat twin
(* of rlsloww. rlsloww opens a file that already exists, so it exercises
(* the two lock-acquire sites; this one CREATES rl.dat, exercising Creat's
(* own unconditional EofLock store (rbf.asm Creat131). By the design a
(* write-only path takes no locks, created or not: a reader should stop
(* at the end that exists. Following the creator is the defect.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i, j: INTEGER
DIM d: STRING[40]
CREATE #path, "rl.dat": WRITE
PRINT #2, "creaw: start v1"
FOR i = 1 TO 5
  d = DATE$
  rec(1) = i
  rec(2) = VAL(MID$(d,13,2))*60+VAL(MID$(d,16,2))
  PUT #path, rec
  PRINT #2, "creaw: wrote record "; i; " at "; rec(2)
  FOR j = 1 TO 10000
  NEXT j
NEXT i
PRINT #2, "creaw: closing"
CLOSE #path
END
