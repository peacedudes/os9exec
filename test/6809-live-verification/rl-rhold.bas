PROCEDURE rlrhold
(* Read-only holder -- the discriminator for the L0BAA record-lock mode gate.
(*
(* Opens rl.dat READ-only, GETs record 0, then holds the path open across a
(* real 3s F$Sleep before closing.  Per the 6809 System Programmers Manual
(* 6.6.1, reading a path opened in read or execute mode must NOT lock the
(* record, because such a path can never update it.
(*
(* Run against rlwait (which opens UPDATE and GETs the same record): if this
(* read-only GET wrongly took a record lock, rlwait blocks for the full sleep.
(* If the mode gate is right, rlwait returns immediately.  That is the only
(* observable difference the gate makes, so it is the only way to test it.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": READ
SEEK #path, 0
GET #path, rec
PRINT #2, "rhold: GET count="; rec(1); " -- read-only path, holding"
regs.x = 600
RUN syscall(10, regs)
PRINT #2, "rhold: closing"
CLOSE #path
END
