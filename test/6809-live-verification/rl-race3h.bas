PROCEDURE rlrace3h
(* rlrace3 with a deliberate hold between GET and PUT, to test whether the
(* automatic record lock serializes at all.  If a READ in update mode locks
(* the record until the next WRITE, then holding it across a real F\$Sleep
(* forces the sibling's GET to block until this PUT releases -- so two racers
(* strictly serialize and NO update is lost.  If losses persist with a wide
(* hold window, the auto-lock is not engaging for these paths (the rapid
(* rlrace3 loss is then not a timing-window artifact but a real lock miss).
(*
(* Same self-report as rlrace3: a run counts when rec(5)=2; then rec(1)=400
(* exactly, or the shortfall is the number of genuinely lost updates.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
OPEN #path, "rl.dat": UPDATE
FOR i = 1 TO 200
  SEEK #path, 0
  GET #path, rec
  regs.x = 2
  RUN syscall(10, regs)
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
PRINT #2, "race3h: finished 200, racers done="; rec(5)
END
