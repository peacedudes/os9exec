e eofinit
 DIM path: BYTE
 DIM rec(5): INTEGER
 DIM i: INTEGER
 FOR i = 1 TO 5
 rec(i) = 0
 NEXT i
 rec(1) = 1
 CREATE #path, "eof.dat": UPDATE
 PUT #path, rec
 CLOSE #path
 PRINT #2, "init: eof.dat created, 1 record, EOF=10"
q
run eofinit
bye
