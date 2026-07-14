! GOTO with a real, explicit numbered line — confirmed WORKING on 68k.
! Supersedes an earlier "NON-FUNCTIONAL" finding that turned out to be a
! methodology mistake: it targeted LIST's hex byte-offset DISPLAY
! annotations (for unnumbered structured code), which are not valid jump
! targets. Classic numbered lines typed directly into source work fine.
!
! LIVE RESULT (68k): i=5
PROCEDURE t_goto
DIM i: INTEGER
i = 0
10 i = i+1
IF i<5 THEN
 GOTO 10
ENDIF
PRINT "i="; i
END
