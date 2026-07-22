PROCEDURE rlrace3g
(* Discriminator: OPEN immediately (concurrent with the companion), but delay
(* 1.5s BEFORE the first GET.  If losses vanish, the timing that matters is the
(* first sector LOAD (buffer coherence), not the OPEN itself (conflict-ring
(* construction).  If losses persist, the open must be staggered to help, so
(* the race is in the open/ring path.  Compare against rlrace3d (delays OPEN).
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
OPEN #path, "rl.dat": UPDATE
regs.x = 90
RUN syscall(10, regs)
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
PRINT #2, "race3g: finished 200, racers done="; rec(5)
END
