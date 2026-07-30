PROCEDURE t12wser
DIM obs, xpc: INTEGER
DIM path, cp: BYTE
DIM rec(5): INTEGER
DIM i, round, fin, go, done: INTEGER
DIM n, place, digit: REAL
DIM digits$: STRING[5]
DIM verdict$: STRING[8]
DIM outp$: STRING[132]
xpc = 1
obs = 0
verdict$ = "ERROR"
GOSUB 3000
ON ERROR GOTO 800
CREATE #path,"SCRATCH/t12.dat":UPDATE
FOR i = 1 TO 5
  rec(i) = 0
NEXT i
rec(1) = 1
PUT #path,rec
CLOSE #path
SHELL "t12whld&"
round = 0
go = 0
done = 0
WHILE done = 0 DO
  round = round + 1
  GOSUB 2500
  IF go = 1 THEN
    done = 1
  ENDIF
  IF round >= 400 THEN
    done = 1
  ENDIF
  IF done = 0 THEN
    FOR i = 1 TO 200
    NEXT i
  ENDIF
ENDWHILE
IF go = 0 THEN 700
ON ERROR GOTO 800
CREATE #cp,"SCRATCH/t12try":WRITE
CLOSE #cp
OPEN #path,"SCRATCH/t12.dat":WRITE
SEEK #path,10
rec(1) = 222
PUT #path,rec
CLOSE #path
ON ERROR
GOSUB 2000
IF fin = 1 THEN
  obs = 1
ELSE
  obs = 0
ENDIF
IF obs = xpc THEN
  verdict$ = "PASS"
ELSE
  verdict$ = "FAIL"
ENDIF
GOTO 820
700 GOSUB 4000
verdict$ = "ERROR"
GOTO 820
800 GOSUB 4000
verdict$ = "ERROR"
820 GOSUB 3000
outp$ = "RESULT t12 " + verdict$ + "  obs="
n = obs
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + " exp="
n = xpc
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + "  a second write-only writer waits at the locked end"
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
2000 fin = 0
ON ERROR GOTO 2100
OPEN #cp,"SCRATCH/t12rel":READ
CLOSE #cp
fin = 1
2100 ON ERROR
RETURN
2500 go = 0
ON ERROR GOTO 2600
OPEN #cp,"SCRATCH/t12go":READ
CLOSE #cp
go = 1
2600 ON ERROR
RETURN
3000 ON ERROR GOTO 3010
DELETE "SCRATCH/t12.dat"
3010 ON ERROR GOTO 3020
DELETE "SCRATCH/t12go"
3020 ON ERROR GOTO 3030
DELETE "SCRATCH/t12try"
3030 ON ERROR GOTO 3040
DELETE "SCRATCH/t12rel"
3040 ON ERROR
RETURN
4000 ON ERROR GOTO 4010
CLOSE #path
4010 ON ERROR
RETURN
