PROCEDURE rldead2
(* Deadlock-detection check, half 2: mirror of rldead1 -- holds record 1 via
(* path1, sleeps 2s, then requests record 0 via path2.  See rl-dead1.bas.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM p1, p2: BYTE
DIM rec(5): INTEGER
DIM e: INTEGER
e = 0
OPEN #p1, "rl.dat": UPDATE
SEEK #p1, 10
GET #p1, rec
PRINT #2, "dead2: holding rec1 via p1"
regs.x = 120
RUN syscall(10, regs)
OPEN #p2, "rl.dat": UPDATE
SEEK #p2, 0
ON ERROR GOTO 500
GET #p2, rec
PRINT #2, "dead2: got rec0, no deadlock seen"
GOTO 600
500 e = ERR
PRINT #2, "dead2: error "; e
600 ON ERROR
CLOSE #p2
CLOSE #p1
END
