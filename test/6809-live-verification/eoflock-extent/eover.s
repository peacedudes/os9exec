e eofover
 DIM path: BYTE
 DIM rec(5): INTEGER
 DIM i: INTEGER
 FOR i = 1 TO 5
 rec(i) = 0
 NEXT i
 OPEN #path, "eof.dat": WRITE
 PRINT #2, "B: about to PUT at 10 (over A's record)"
 SEEK #path, 10
 rec(1) = 222
 PUT #path, rec
 PRINT #2, "B: PUT returned"
 CLOSE #path
q
run eofover
bye
