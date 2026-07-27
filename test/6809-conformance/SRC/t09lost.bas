PROCEDURE t09lost
DIM obs, xpc: INTEGER
DIM path, cp: BYTE
DIM rec(5): INTEGER
DIM i, round, both, done: INTEGER
DIM n, place, digit: REAL
DIM digits$: STRING[5]
DIM verdict$: STRING[8]
DIM outp$: STRING[132]
xpc = 200
obs = 0
verdict$ = "ERROR"
GOSUB 3000
ON ERROR GOTO 800
CREATE #path,"SCRATCH/t09.dat":UPDATE
FOR i = 1 TO 5
  rec(i) = 0
NEXT i
PUT #path,rec
PUT #path,rec
CLOSE #path
OPEN #path,"SCRATCH/t09.dat":READ
GET #path,rec
IF rec(1) <> 0 THEN 700
GET #path,rec
CLOSE #path
ON ERROR
SHELL "t09race& t09race&"
both = 0
round = 0
done = 0
WHILE done = 0 DO
  round = round + 1
  GOSUB 2000
  IF both = 1 THEN
    done = 1
  ENDIF
  IF round >= 200 THEN
    done = 1
  ENDIF
  IF done = 0 THEN
    FOR i = 1 TO 400
    NEXT i
  ENDIF
ENDWHILE
IF both = 0 THEN 700
ON ERROR GOTO 800
OPEN #path,"SCRATCH/t09.dat":READ
GET #path,rec
CLOSE #path
ON ERROR
obs = rec(1)
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
outp$ = "RESULT t09 " + verdict$ + "  obs="
n = obs
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + " exp="
n = xpc
GOSUB 1000
outp$ = outp$ + digits$
outp$ = outp$ + "  concurrent read-modify-write on one record loses no update"
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
2000 both = 0
ON ERROR GOTO 2100
OPEN #cp,"SCRATCH/t09d1":READ
CLOSE #cp
OPEN #cp,"SCRATCH/t09d2":READ
CLOSE #cp
both = 1
2100 ON ERROR
RETURN
3000 ON ERROR GOTO 3010
DELETE "SCRATCH/t09.dat"
3010 ON ERROR GOTO 3020
DELETE "SCRATCH/t09c1"
3020 ON ERROR GOTO 3030
DELETE "SCRATCH/t09c2"
3030 ON ERROR GOTO 3040
DELETE "SCRATCH/t09d1"
3040 ON ERROR GOTO 3050
DELETE "SCRATCH/t09d2"
3050 ON ERROR
RETURN
4000 ON ERROR GOTO 4010
CLOSE #path
4010 ON ERROR
RETURN
