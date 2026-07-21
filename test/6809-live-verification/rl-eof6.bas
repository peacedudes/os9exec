PROCEDURE rleof6
(* Untrapped death at EOF on an UPDATE path. rleof3 (READ) dies with the
(* banner saying 203; the only 203 in RBF is SetStat SS.Size on a path
(* lacking WRITE., which would SUCCEED silently here. If this banner says
(* 211, the mystery operation is mode-dependent and SS.Size fits; check
(* the file size afterwards, because a successful SS.Size here means the
(* abort truncated a file it had no business touching.
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": UPDATE
PRINT #2, "eof6: opened v1 UPDATE untrapped"
GET #path, rec
PRINT #2, "eof6: got record, counter="; rec(1)
GET #path, rec
PRINT #2, "eof6: got second record, counter="; rec(1)
CLOSE #path
END
