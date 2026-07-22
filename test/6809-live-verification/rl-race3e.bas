PROCEDURE rlrace3e
(* Companion to the E\$Lock probe RBF (a conflict returns Error 252 instead of
(* sleeping).  Retries the GET on a lock error, yielding a tick each try, so
(* serialization happens in the guest, not in RBF's sleep/wake.  If losses go to
(* 0, RBF's conflict DETECTION is complete and correct and the fault is RBF's
(* own sleep/wake/reacquire; if losses persist, detection misses some overlaps.
(* Pure GOTO layout -- BASIC09 here rejects FOR mixed with ON ERROR GOTO.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM rec(5): INTEGER
DIM i, retries: INTEGER
OPEN #path, "rl.dat": UPDATE
retries = 0
i = 1
ON ERROR GOTO 500
5 SEEK #path, 0
GET #path, rec
rec(1) = rec(1) + 1
SEEK #path, 0
PUT #path, rec
i = i + 1
IF i <= 200 THEN 5
GOTO 600
500 retries = retries + 1
regs.x = 1
RUN syscall(10, regs)
GOTO 5
600 ON ERROR
SEEK #path, 0
GET #path, rec
rec(5) = rec(5) + 1
SEEK #path, 0
PUT #path, rec
CLOSE #path
PRINT #2, "race3e: done="; rec(5); " retries="; retries
END
