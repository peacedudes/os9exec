! GOSUB with a real, explicit numbered line, including an inline
! subroutine body reached only via GOSUB (skipped over via GOTO in normal
! flow) — confirmed WORKING on 68k. See basic09-goto-test.bas for the
! GOTO-only counterpart and its superseded "NON-FUNCTIONAL" history.
!
! LIVE RESULT (68k): "after gosub i=10" then "done i=10"
PROCEDURE t_gosub
DIM i: INTEGER
i = 0
GOSUB 100
PRINT "after gosub i="; i
GOTO 200
100 i = i+10
RETURN
200 PRINT "done i="; i
END
