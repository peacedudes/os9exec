PROCEDURE t12whld
DIM path, cp: BYTE
DIM rec(5): INTEGER
DIM i, j, round, seen, done: INTEGER
FOR i = 1 TO 5
  rec(i) = 0
NEXT i
OPEN #path,"SCRATCH/t12.dat":WRITE
SEEK #path,10
rec(1) = 111
PUT #path,rec
CREATE #cp,"SCRATCH/t12go":WRITE
CLOSE #cp
round = 0
seen = 0
done = 0
WHILE done = 0 DO
  round = round + 1
  GOSUB 2500
  IF seen = 1 THEN
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
FOR j = 1 TO 20000
NEXT j
CREATE #cp,"SCRATCH/t12rel":WRITE
CLOSE #cp
SEEK #path,0
rec(1) = 99
PUT #path,rec
CLOSE #path
END
2500 seen = 0
ON ERROR GOTO 2600
OPEN #cp,"SCRATCH/t12try":READ
CLOSE #cp
seen = 1
2600 ON ERROR
RETURN
