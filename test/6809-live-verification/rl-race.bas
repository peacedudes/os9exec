PROCEDURE rlrace
(* The lost-update race -- the headline test for RBF record locking.
(*
(* Two copies of this run concurrently, each adding 1 to the same record
(* 200 times. There is deliberately no SS.Lock call anywhere: the design
(* says a GET on an update-mode path locks the record and the following
(* PUT releases it, which is exactly what makes an unprotected
(* read-modify-write cycle safe. So the total must be 400.
(*
(* Anything less means two processes read the same value and one of the
(* increments was overwritten -- a lost update.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
OPEN #path, "rl.dat": UPDATE
FOR i = 1 TO 200
  SEEK #path, 0
  GET #path, rec
  rec(1) = rec(1) + 1
  SEEK #path, 0
  PUT #path, rec
NEXT i
CLOSE #path
PRINT #2, "race: this racer finished its 200"
END
