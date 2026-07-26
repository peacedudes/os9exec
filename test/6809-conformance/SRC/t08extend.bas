PROCEDURE t08extend
DIM obs, xpc, count: INTEGER
DIM path: BYTE
DIM seekpos: REAL
DIM mkb, rb: BYTE
DIM n, place, digit, i: REAL
DIM digits$: STRING[5]
DIM verdict$: STRING[8]
DIM outp$: STRING[132]
xpc = 54
obs = -1
seekpos = 50.
ON ERROR GOTO 90
DELETE "SCRATCH/t08extend.dat"
90 ON ERROR
ON ERROR GOTO 800
CREATE #path,"SCRATCH/t08extend.dat":WRITE
SEEK #path,seekpos
mkb = 69
PUT #path,mkb
mkb = 78
PUT #path,mkb
mkb = 68
PUT #path,mkb
mkb = 33
PUT #path,mkb
CLOSE #path
OPEN #path,"SCRATCH/t08extend.dat":READ
count = 0
ON ERROR GOTO 700
LOOP
  GET #path,rb
  count = count + 1
ENDLOOP
700 ON ERROR
CLOSE #path
obs = count
GOTO 820
800 obs = ERR
ON ERROR
820 DELETE "SCRATCH/t08extend.dat"
IF obs = xpc THEN
  verdict$ = "PASS"
ELSE
  verdict$ = "FAIL"
ENDIF
outp$ = "RESULT t08 " + verdict$ + "  obs="
n = obs
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + " exp="
n = xpc
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + "  a write past a seek beyond EOF extends the file"
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
