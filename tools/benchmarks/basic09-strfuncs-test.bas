! String function coverage beyond LEN/MID$/SUBSTR (which basic09-langtest
! already tests). All confirmed matching the documented behavior on 68k.
! TRIM$ is the interesting one: it strips TRAILING spaces only, not both
! sides — TRIM$("  hi  ")="  hi" (leading spaces survive). An earlier
! version of this test wrongly expected "hi"; fixed to match the manual
! ("removes trailing spaces") and the live result.
!
! LIVE RESULT (68k):
!   PASS: LEFT$ / RIGHT$ / STR$ / VAL / CHR$ / ASC / TRIM$ (trailing spaces only)
!   Total: 7  Pass: 7  Fail: 0
PROCEDURE t_strfuncs
DIM s: STRING[20]
DIM r$: STRING[10]
DIM n: INTEGER
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
s = "Hello World"
total = total+1
IF LEFT$(s,5)="Hello" THEN
 pass = pass+1
 PRINT "PASS: LEFT$"
ELSE
 fail = fail+1
 PRINT "FAIL: LEFT$ got "; LEFT$(s,5)
ENDIF
total = total+1
IF RIGHT$(s,5)="World" THEN
 pass = pass+1
 PRINT "PASS: RIGHT$"
ELSE
 fail = fail+1
 PRINT "FAIL: RIGHT$ got "; RIGHT$(s,5)
ENDIF
total = total+1
r$ = STR$(42)
IF r$="42" THEN
 pass = pass+1
 PRINT "PASS: STR$"
ELSE
 fail = fail+1
 PRINT "FAIL: STR$ got "; r$
ENDIF
total = total+1
n = VAL("123")
IF n=123 THEN
 pass = pass+1
 PRINT "PASS: VAL"
ELSE
 fail = fail+1
 PRINT "FAIL: VAL got "; n
ENDIF
total = total+1
r$ = CHR$(65)
IF r$="A" THEN
 pass = pass+1
 PRINT "PASS: CHR$"
ELSE
 fail = fail+1
 PRINT "FAIL: CHR$ got "; r$
ENDIF
total = total+1
n = ASC("A")
IF n=65 THEN
 pass = pass+1
 PRINT "PASS: ASC"
ELSE
 fail = fail+1
 PRINT "FAIL: ASC got "; n
ENDIF
total = total+1
r$ = TRIM$("  hi  ")
IF r$="  hi" THEN
 pass = pass+1
 PRINT "PASS: TRIM$ (trailing spaces only)"
ELSE
 fail = fail+1
 PRINT "FAIL: TRIM$ got ["; r$; "]"
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
