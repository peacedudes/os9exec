PROCEDURE rlappend
(* Companion to rlgrow -- appends nine records to rl.dat, one at a time,
(* so the file grows from one record to ten while rlgrow holds it open.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
DIM j: INTEGER
OPEN #path, "rl.dat": UPDATE
SEEK #path, 10
FOR i = 2 TO 10
  rec(1) = i
  PUT #path, rec
  PRINT #2, "append: wrote record "; i
  FOR j = 1 TO 3000
  NEXT j
NEXT i
PRINT #2, "append: done, closing"
CLOSE #path
END
