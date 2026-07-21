PROCEDURE rleof7
(* Does runb's untrapped abort TRUNCATE an update-mode file?
(* Run against a 60-byte rl.dat (rlinit + rlsloww). Reads one record so
(* the position sits at 10 with 50 bytes beyond it, then dies untrapped
(* on a divide by zero -- no I/O error involved. If the abort issues
(* SS.Size at the current position, the file comes back 10 bytes long
(* and the abort path is destroying data; if it stays 60, it does not.
DIM path: BYTE
DIM rec(5): INTEGER
DIM x: INTEGER
OPEN #path, "rl.dat": UPDATE
PRINT #2, "eof7: opened v1 UPDATE, file should be 60 bytes"
GET #path, rec
PRINT #2, "eof7: got record, position now 10"
x = 0
x = 1/x
PRINT #2, "eof7: unreachable"
CLOSE #path
END
