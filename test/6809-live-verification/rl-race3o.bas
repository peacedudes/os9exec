PROCEDURE rlrace3o
(* Observation racer: same loop I/O as rlrace3 (SEEK/GET/+1/SEEK/PUT on the
(* one shared record), but each GET's full record is logged IN MEMORY and each
(* PUT stamps rec(2)=my id, rec(3)=my iteration inside the same 10 bytes.  The
(* log dumps to a per-racer file only after the race, so the contended loop is
(* observed without adding a single syscall to it.  Identity self-assigns via
(* rec(4); a collision makes the second CREATE fail, killing the run visibly.
DIM path, lp: BYTE
DIM rec(5): INTEGER
DIM lv(200), lw(200), lj(200): INTEGER
DIM i, myid: INTEGER
OPEN #path, "rl.dat": UPDATE
SEEK #path, 0
GET #path, rec
myid = rec(4) + 1
rec(4) = myid
SEEK #path, 0
PUT #path, rec
i = 1
5 SEEK #path, 0
GET #path, rec
lv(i) = rec(1)
lw(i) = rec(2)
lj(i) = rec(3)
rec(1) = rec(1) + 1
rec(2) = myid
rec(3) = i
SEEK #path, 0
PUT #path, rec
i = i + 1
IF i <= 200 THEN 5
SEEK #path, 0
GET #path, rec
rec(5) = rec(5) + 1
SEEK #path, 0
PUT #path, rec
CLOSE #path
IF myid = 2 THEN 50
CREATE #lp, "log1": WRITE
GOTO 60
50 CREATE #lp, "log2": WRITE
60 PUT #lp, lv
PUT #lp, lw
PUT #lp, lj
CLOSE #lp
PRINT #2, "race3o: id="; myid; " finished 200, racers done="; rec(5)
END
