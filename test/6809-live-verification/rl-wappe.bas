PROCEDURE rlwappe
(* Write-only twin of rlappend: opens with WRITE, not UPDATE.
(* so the file grows from one record to ten while rlgrow holds it open.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
DIM j: INTEGER
OPEN #path, "rl.dat": WRITE
SEEK #path, 10
FOR i = 2 TO 10
  rec(1) = i
  PUT #path, rec
  PRINT #2, "wappend: wrote record "; i
  FOR j = 1 TO 3000
  NEXT j
NEXT i
PRINT #2, "wappend: done, closing"
CLOSE #path
END
