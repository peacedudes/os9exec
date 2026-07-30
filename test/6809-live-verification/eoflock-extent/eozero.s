e eozero
 DIM path: BYTE
 DIM rec(5): INTEGER
 DIM i: INTEGER
 FOR i = 1 TO 5
 rec(i) = 0
 NEXT i
 OPEN #path, "eof.dat": WRITE
 PRINT #2, "B: about to PUT at 0 (well before A, not at EOF)"
 SEEK #path, 0
 rec(1) = 222
 PUT #path, rec
 PRINT #2, "B: PUT returned"
 CLOSE #path
q
run eozero
bye
