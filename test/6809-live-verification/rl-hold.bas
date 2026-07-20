PROCEDURE rlhold
(* Does a WRITE release the record lock a READ took?
(*
(* The design says a read on an update-mode path locks the record it read,
(* and the NEXT WRITE on that path releases it -- that release is what makes
(* an unprotected read-modify-write cycle safe. This program holds the lock,
(* releases it with a PUT, and then deliberately stays open doing nothing
(* for a long time before closing.
(*
(* rlwait runs concurrently and blocks on the same record. Where its
(* "returned" marker lands decides the question:
(*   after "hold: PUT done"   -> the write released the lock (design honoured)
(*   after "hold: closing"    -> only CLOSE released it (the write did not)
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
OPEN #path, "rl.dat": UPDATE
SEEK #path, 0
GET #path, rec
PRINT #2, "hold: GET done, count="; rec(1); " -- lock held"
FOR i = 1 TO 12000
NEXT i
rec(1) = rec(1) + 1
SEEK #path, 0
PUT #path, rec
PRINT #2, "hold: PUT done, count="; rec(1); " -- lock should now be free"
FOR i = 1 TO 24000
NEXT i
PRINT #2, "hold: closing"
CLOSE #path
END
