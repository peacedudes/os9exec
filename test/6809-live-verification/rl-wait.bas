PROCEDURE rlwait
(* Companion to rlhold -- see that file for what the pair proves.
(* Opens the same record in update mode and reads it. If rlhold's read
(* really locked the record, this GET must block; when it returns is the
(* whole measurement, so the markers around it are the result.
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": UPDATE
PRINT #2, "wait: about to GET"
SEEK #path, 0
GET #path, rec
PRINT #2, "wait: GET returned, count="; rec(1)
CLOSE #path
END
