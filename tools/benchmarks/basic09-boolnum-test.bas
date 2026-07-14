! Does using a BOOLEAN in a numeric expression raise a runtime error, as
! assumed? NO — it's a COMPILE-TIME error, caught by LOAD before the
! program can even run. Confirmed on 68k: both `flag + 1` lines below
! (flag: BOOLEAN) fail to compile with `Error #000:067 (E_ILLARG) illegal
! argument`, one per occurrence — LOAD reports both, and RUN just
! re-surfaces the same failure since the program never compiled.
!
! LIVE RESULT (68k): LOAD reports two E_ILLARG errors, RUN never executes
PROCEDURE t_boolnum
DIM flag: BOOLEAN
DIM n: INTEGER
flag = TRUE
n = flag + 1
PRINT "n="; n
flag = FALSE
n = flag + 1
PRINT "n="; n
END
