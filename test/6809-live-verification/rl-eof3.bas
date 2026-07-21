PROCEDURE rleof3
(* Untrapped twin of rleof1: no ON ERROR, so the GET at end of file kills
(* the program and whatever error code escapes is what the shell reports.
(* rleof1 proves the GET itself receives 211; if this run's death banner
(* says 203, the change happens in runb's untrapped-abort path, not in
(* RBF or IOMan -- the kernel's answer was correct all along.
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": READ
PRINT #2, "eof3: opened v1 READ untrapped"
GET #path, rec
PRINT #2, "eof3: got record, counter="; rec(1)
GET #path, rec
PRINT #2, "eof3: got record, counter="; rec(1)
PRINT #2, "eof3: no error at all"
CLOSE #path
END
