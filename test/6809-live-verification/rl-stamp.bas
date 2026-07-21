PROCEDURE rlstamp
(* Prints a date stamp to stdout once a second, pacing with a real
(* F\$Sleep (code 10, ticks in X, 60/s) via BASIC09's syscall module
(* instead of a busy delay loop.
TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
DIM regs: registers
DIM i: INTEGER
FOR i = 1 TO 10
  PRINT i; " "; DATE$
  regs.x = 60
  RUN syscall(10, regs)
NEXT i
END
