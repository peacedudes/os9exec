PROCEDURE rldead1
(* Deadlock-detection check, half 1.  A conflicted claim releases that path's
(* own locks before parking, so a cycle needs hold-and-wait across TWO paths:
(* hold record 0 via path1, sleep 2s so the mirror (rldead2) takes record 1,
(* then request record 1 via path2 -- path1's hold survives the park.  On a
(* 20-byte rl.dat (rlinit2), the crossed pair must produce exactly one
(* E$DeadLk (#254); the other half completes after the loser closes.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM p1, p2: BYTE
DIM rec(5): INTEGER
DIM e: INTEGER
e = 0
OPEN #p1, "rl.dat": UPDATE
SEEK #p1, 0
GET #p1, rec
PRINT #2, "dead1: holding rec0 via p1"
regs.x = 120
RUN syscall(10, regs)
OPEN #p2, "rl.dat": UPDATE
SEEK #p2, 10
ON ERROR GOTO 500
GET #p2, rec
PRINT #2, "dead1: got rec1, no deadlock seen"
GOTO 600
500 e = ERR
PRINT #2, "dead1: error "; e
600 ON ERROR
CLOSE #p2
CLOSE #p1
END
