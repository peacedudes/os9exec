PROCEDURE rleof2
(* Twin of rleof1 with the path opened UPDATE instead of READ.
(* Separates "reads at EOF report the wrong error" from "read-only paths
(* report the wrong error": if this one sees 211 where rleof1 sees 203,
(* the mode of the path changes the error a read at EOF returns.
DIM path: BYTE
DIM rec(5): INTEGER
DIM n: INTEGER
ON ERROR GOTO 100
n = 0
OPEN #path, "rl.dat": UPDATE
PRINT #2, "eof2: opened v1 UPDATE"
n = 1
GET #path, rec
PRINT #2, "eof2: got record, counter="; rec(1)
n = 2
GET #path, rec
PRINT #2, "eof2: got record, counter="; rec(1)
n = 3
GET #path, rec
PRINT #2, "eof2: got record, counter="; rec(1)
PRINT #2, "eof2: no error at all"
CLOSE #path
END
100 PRINT #2, "eof2: ERR="; ERR; " at step "; n
CLOSE #path
END
