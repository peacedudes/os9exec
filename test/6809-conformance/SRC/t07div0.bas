PROCEDURE t07div0
DIM obs, xpc: INTEGER
DIM divd, divs: INTEGER
DIM n, place, digit, i: REAL
DIM digits$: STRING[5]
DIM verdict$: STRING[8]
DIM outp$: STRING[132]
xpc = 45
obs = 0
divd = 1
divs = 0
ON ERROR GOTO 500
obs = divd / divs
ON ERROR
GOTO 600
500 obs = ERR
ON ERROR
600 IF obs = xpc THEN
  verdict$ = "PASS"
ELSE
  verdict$ = "FAIL"
ENDIF
outp$ = "RESULT t07 " + verdict$ + "  obs="
n = obs
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + " exp="
n = xpc
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + "  INTEGER divide by zero traps into BASIC09's own error 45"
PRINT outp$
END
1000 IF n < 0. THEN
  n = n + 65536.
ENDIF
digits$ = ""
place = 10000.
FOR i = 1 TO 5
  digit = 0.
  WHILE n >= place DO
    n = n - place
    digit = digit + 1.
  ENDWHILE
  digits$ = digits$ + CHR$(48+digit)
  place = place / 10.
NEXT i
RETURN
