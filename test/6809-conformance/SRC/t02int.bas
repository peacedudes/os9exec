PROCEDURE t02int
DIM obs, xpc: INTEGER
DIM n, place, digit, i: REAL
DIM digits$: STRING[5]
DIM verdict$: STRING[8]
DIM outp$: STRING[132]
xpc = -32768
obs = 32767
obs = obs + 1
IF obs = xpc THEN
  verdict$ = "PASS"
ELSE
  verdict$ = "FAIL"
ENDIF
outp$ = "RESULT t02 " + verdict$ + "  obs="
n = obs
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + " exp="
n = xpc
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + "  INTEGER overflow on 6809 wraps modulo 65536 (32767+1=-32768)"
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
