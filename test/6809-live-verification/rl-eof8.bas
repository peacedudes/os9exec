PROCEDURE rleof8
(* The smoking-gun probe for the abort-path SS.Size call.
(* Run against a 60-byte rl.dat. Seeks to byte 100 -- past the end, which
(* is legal -- and dies untrapped on the GET there. If runb's I/O abort
(* issues SS.Size at the current position on this UPDATE path, the file
(* comes back EXTENDED to 100 bytes; rlfollo then counts 10 records where
(* there were 6. A read-mode twin dies 203 with the file untouched.
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": UPDATE
PRINT #2, "eof8: opened v1 UPDATE, seeking to 100 of 60"
SEEK #path, 100
GET #path, rec
PRINT #2, "eof8: unreachable, GET at 100 succeeded"
CLOSE #path
END
