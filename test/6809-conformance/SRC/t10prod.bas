PROCEDURE t10prod
DIM path, cp: BYTE
DIM rec(5): INTEGER
DIM i, j: INTEGER
FOR i = 1 TO 5
  rec(i) = 0
NEXT i
OPEN #path,"SCRATCH/t10.dat":UPDATE
SEEK #path,10
rec(1) = 2
PUT #path,rec
CREATE #cp,"SCRATCH/t10go":WRITE
CLOSE #cp
FOR i = 3 TO 7
  FOR j = 1 TO 10000
  NEXT j
  rec(1) = i
  PUT #path,rec
NEXT i
CLOSE #path
CREATE #cp,"SCRATCH/t10dn":WRITE
CLOSE #cp
END
