PROCEDURE rlcheck
(* Reports the counter left by the rlrace pair. 400 means no update was
(* lost; anything less is the count of increments that vanished.
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": READ
SEEK #path, 0
GET #path, rec
CLOSE #path
PRINT #2, "check: counter="; rec(1); " of 400 expected, lost="; 400 - rec(1)
END
