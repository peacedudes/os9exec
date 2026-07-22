PROCEDURE rlqhold
(* Quick holder for the concurrent-open ring-split test.  Opens rl.dat,
(* GETs record 0 (auto-locks it in update mode), holds it across a real 3s
(* F\$Sleep, then increments and PUTs (releasing).  Launched concurrently with
(* rlwait: if the two opens land on the same conflict ring, rlwait's GET blocks
(* until this PUT and reads count=1; if concurrent opens split the ring, rlwait
(* never sees this lock and reads count=0.  That distinguishes a ring-split
(* from a buffer-coherence cause for the lost updates.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": UPDATE
SEEK #path, 0
GET #path, rec
PRINT #2, "qhold: GET count="; rec(1); " -- lock held"
regs.x = 180
RUN syscall(10, regs)
rec(1) = rec(1) + 1
SEEK #path, 0
PUT #path, rec
PRINT #2, "qhold: PUT count="; rec(1); " -- released"
CLOSE #path
END
