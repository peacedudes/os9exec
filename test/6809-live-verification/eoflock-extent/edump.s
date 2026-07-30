e eofdump
 DIM path: BYTE
 DIM rec(5): INTEGER
 DIM i: INTEGER
 OPEN #path, "eof.dat": READ
 ON ERROR GOTO 100
 FOR i = 1 TO 4
 GET #path, rec
 PRINT #2, "rec"; i; " ="; rec(1)
 NEXT i
 100 ON ERROR
 CLOSE #path
q
run eofdump
bye
