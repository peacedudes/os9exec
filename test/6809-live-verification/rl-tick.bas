PROCEDURE rltick
(* Date-stamps a line each second for 60 seconds into a text file open
(* for update, so a concurrent `list` chases it, parked on the eof lock.
(* Paces with a real F\$Sleep (code 10, ticks in X, 60/s), and deletes
(* any leftover from a previous run itself: a rerun is just rltick.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM path: BYTE
DIM i: INTEGER
ON ERROR GOTO 10
DELETE "rltick.txt"
10 ON ERROR
CREATE #path, "rltick.txt": UPDATE
PRINT #2, "tick: start v3"
FOR i = 1 TO 60
  PRINT #path, "line "; i; " at "; DATE$
  regs.x = 60
  RUN syscall(10, regs)
NEXT i
PRINT #2, "tick: closing"
CLOSE #path
END
