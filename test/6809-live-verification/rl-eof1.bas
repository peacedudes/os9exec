PROCEDURE rleof1
(* What error does a read-only GET see at end of file?
(* rl.dat is one 10-byte record after rlinit, so the first GET succeeds
(* and the second lands exactly at EOF. The design says that read should
(* report end-of-file (211); Error #203 - Illegal Mode is the defect
(* under investigation. ON ERROR captures the code the program really
(* receives, separating the kernel's answer from runb's death report.
DIM path: BYTE
DIM rec(5): INTEGER
DIM n: INTEGER
ON ERROR GOTO 100
n = 0
OPEN #path, "rl.dat": READ
PRINT #2, "eof1: opened v1 READ"
n = 1
GET #path, rec
PRINT #2, "eof1: got record, counter="; rec(1)
n = 2
GET #path, rec
PRINT #2, "eof1: got record, counter="; rec(1)
n = 3
GET #path, rec
PRINT #2, "eof1: got record, counter="; rec(1)
PRINT #2, "eof1: no error at all"
CLOSE #path
END
100 PRINT #2, "eof1: ERR="; ERR; " at step "; n
CLOSE #path
END
