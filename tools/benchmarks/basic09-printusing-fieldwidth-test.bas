! PRINT USING field-width verification, captured precisely via a file
! round-trip + LEN() (not by eyeballing terminal spacing, which can lose
! trailing spaces). Two findings:
! 1. R8.2 with 12.349 produces an EXACTLY 8-character field, correctly
!    rounded to 12.35 -- field-width math confirmed exact.
! 2. REAL DIVERGENCE FROM THE MANUAL, not a test mistake: the manual
!    states BOOLEAN format converts to "TRUE"/"FALSE" (uppercase).
!    Live-verified: B8 with TRUE prints "True    " -- mixed case (capital
!    T only), not "TRUE". Field width is still exactly correct (8 chars).
!    Worth flagging to the compiler's author directly.
!
! NOTE: the path-number placement syntax was also wrong on the first two
! attempts before this file -- PRINT USING #path is INVALID (Error
! #000:018); the correct form is PRINT #path USING "fmt", list (path
! comes right after PRINT, before USING).
!
! LIVE RESULT (68k): 1/2 PASS -- the 1 "FAIL" is the confirmed real
! divergence (#2 above), not a test bug.
PROCEDURE t_printusing4
DIM r: REAL
DIM b: BOOLEAN
DIM path: BYTE
DIM out: STRING[20]
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
CREATE #path, "putest2": WRITE
r = 12.349
PRINT #path USING "R8.2", r
b = TRUE
PRINT #path USING "B8", b
CLOSE #path
OPEN #path, "putest2": READ
READ #path, out
total = total+1
IF LEN(out)=8 THEN
 pass = pass+1
 PRINT "PASS: R8.2 field width exactly 8, content=["; out; "]"
ELSE
 fail = fail+1
 PRINT "FAIL: R8.2 LEN="; LEN(out); " content=["; out; "]"
ENDIF
READ #path, out
total = total+1
IF out="TRUE    " THEN
 pass = pass+1
 PRINT "PASS: B8 TRUE prints uppercase TRUE, content=["; out; "]"
ELSE
 fail = fail+1
 PRINT "FAIL: B8 TRUE content=["; out; "] LEN="; LEN(out)
ENDIF
CLOSE #path
DELETE "putest2"
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
