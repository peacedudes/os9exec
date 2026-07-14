! DATA/READ/RESTORE — confirmed on 68k: READ pulls values across DATA
! statement boundaries in declaration order; a bare RESTORE rewinds to
! the very first DATA value; and READing past the last DATA value wraps
! around to the first one again rather than erroring (documented
! behavior, confirmed live: after RESTORE + one READ, the pointer sits
! at the 2nd of 4 total values — reading 4 more consumes values 2,3,4
! then wraps to value 1).
!
! LIVE RESULT (68k): all 4 checks PASS
PROCEDURE t_dataread2
DIM a,b,c: INTEGER
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
READ a,b,c
total = total+1
IF a=1 AND b=2 AND c=3 THEN
 pass = pass+1
 PRINT "PASS: READ pulls DATA in order"
ELSE
 fail = fail+1
 PRINT "FAIL: a="; a; " b="; b; " c="; c
ENDIF
READ a
total = total+1
IF a=4 THEN
 pass = pass+1
 PRINT "PASS: READ continues past first exhausted line"
ELSE
 fail = fail+1
 PRINT "FAIL: a="; a
ENDIF
RESTORE
READ a
total = total+1
IF a=1 THEN
 pass = pass+1
 PRINT "PASS: bare RESTORE rewinds to first DATA"
ELSE
 fail = fail+1
 PRINT "FAIL: a="; a
ENDIF
READ a,b,c,a
total = total+1
IF a=1 THEN
 pass = pass+1
 PRINT "PASS: DATA wraps around when exhausted (4th value wraps to 1st), a="; a
ELSE
 fail = fail+1
 PRINT "FAIL: wraparound a="; a
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
DATA 1,2,3
DATA 4
