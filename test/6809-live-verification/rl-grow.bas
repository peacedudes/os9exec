PROCEDURE rlgrow
(* Does a reader see end-of-file move as a writer appends?
(*
(* Opens rl.dat while it is still one record long, then polls: each round
(* rewinds, reads as far as it can, and reports the count. rlappend runs
(* concurrently, appending nine records with a pause between each.
(*
(* A rising count across the rounds means the reader tracks the file as it
(* grows. A count stuck at 1 until the final round means the reader only
(* ever learns the new size once the writer has closed.
DIM path: BYTE
DIM rec(5): INTEGER
DIM n: INTEGER
DIM i: INTEGER
DIM round: INTEGER
OPEN #path, "rl.dat": READ
PRINT #2, "grow: opened while file is 1 record"
FOR round = 1 TO 6
  n = 0
  ON ERROR GOTO 100
  SEEK #path, 0
  WHILE n < 10 DO
    GET #path, rec
    n = n + 1
  ENDWHILE
100 ON ERROR
  PRINT #2, "grow: round "; round; " sees "; n; " records"
  FOR i = 1 TO 5000
  NEXT i
NEXT round
CLOSE #path
END
