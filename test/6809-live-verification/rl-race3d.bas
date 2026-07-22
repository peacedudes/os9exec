PROCEDURE rlrace3d
(* rlrace3 with a deliberate 1.5s delay BEFORE the OPEN, to stagger the two
(* racers' opens.  If lost updates come from a race in RBF's conflict-ring
(* construction (two concurrent opens landing on separate rings, so record
(* locking never sees the sibling), then opening well after the companion --
(* which is already established on the ring -- should let locking engage and
(* drive losses to ~0.  Run this as the foreground racer against a plain
(* rlrace3 backgrounded (which opens immediately).
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
regs.x = 90
RUN syscall(10, regs)
OPEN #path, "rl.dat": UPDATE
FOR i = 1 TO 200
  SEEK #path, 0
  GET #path, rec
  rec(1) = rec(1) + 1
  SEEK #path, 0
  PUT #path, rec
NEXT i
SEEK #path, 0
GET #path, rec
rec(5) = rec(5) + 1
SEEK #path, 0
PUT #path, rec
CLOSE #path
PRINT #2, "race3d: finished 200, racers done="; rec(5)
END
