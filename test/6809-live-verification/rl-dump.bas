PROCEDURE rldump
(* Prints both racers' in-memory GET logs (written by rlrace3o) as parseable
(* lines: Ln,i,value,writer,writer-iter.  Run only after both racers finish.
DIM lp: BYTE
DIM lv(200), lw(200), lj(200): INTEGER
DIM i: INTEGER
OPEN #lp, "log1": READ
GET #lp, lv
GET #lp, lw
GET #lp, lj
CLOSE #lp
FOR i = 1 TO 200
PRINT #2, "L1,"; i; ","; lv(i); ","; lw(i); ","; lj(i)
NEXT i
OPEN #lp, "log2": READ
GET #lp, lv
GET #lp, lw
GET #lp, lj
CLOSE #lp
FOR i = 1 TO 200
PRINT #2, "L2,"; i; ","; lv(i); ","; lw(i); ","; lj(i)
NEXT i
END
