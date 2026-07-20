PROCEDURE rlsloww
(* Write-only producer, one record roughly every second, each stamped with
(* the time it was written.
(*
(* The pace is the point. A producer running flat out leaves the reader
(* hopelessly behind, and then nothing about locking is being tested at
(* all -- the reader simply never reaches the end of the file.
(*
(* Opened WRITE, so by the design it takes no eof lock, and a reader should
(* stop at the end that exists rather than follow along behind.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i, j: INTEGER
DIM d: STRING[40]
OPEN #path, "rl.dat": WRITE
SEEK #path, 10
FOR i = 2 TO 6
  d = DATE$
  rec(1) = i
  rec(2) = VAL(MID$(d,13,2)) * 60 + VAL(MID$(d,16,2))
  PUT #path, rec
  PRINT #2, "sloww: wrote record "; i; " at "; rec(2)
  FOR j = 1 TO 10000
  NEXT j
NEXT i
PRINT #2, "sloww: closing"
CLOSE #path
END
