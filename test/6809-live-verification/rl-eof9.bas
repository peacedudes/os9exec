PROCEDURE rleof9
(* Trapped twin of rleof8: same UPDATE-mode seek to 100 of a 60-byte
(* file, same failing GET, but with ON ERROR so the program survives.
(* If the file still grows, the SS.Size call belongs to GET's own error
(* handling and fires on every failed read; if it stays 60 bytes, the
(* call happens only in runb's untrapped-abort path.
DIM path: BYTE
DIM rec(5): INTEGER
ON ERROR GOTO 100
OPEN #path, "rl.dat": UPDATE
PRINT #2, "eof9: opened v1 UPDATE, seeking to 100 of 60"
SEEK #path, 100
GET #path, rec
PRINT #2, "eof9: unreachable, GET at 100 succeeded"
CLOSE #path
END
100 PRINT #2, "eof9: ERR="; ERR; " trapped, closing normally"
CLOSE #path
END
