PROCEDURE rleof5
(* Traps the EOF like rleof1 but ENDs without closing the path, leaving
(* the close to runb's own exit cleanup. If the shell reports an error
(* after this program's own output, the failing operation lives in that
(* cleanup; if the run ends silently, it lives in the abort path alone.
DIM path: BYTE
DIM rec(5): INTEGER
ON ERROR GOTO 100
OPEN #path, "rl.dat": READ
PRINT #2, "eof5: opened v1 READ, will not close"
GET #path, rec
PRINT #2, "eof5: got record, counter="; rec(1)
GET #path, rec
PRINT #2, "eof5: got second record, counter="; rec(1)
END
100 PRINT #2, "eof5: ERR="; ERR; " ending with path open"
END
