e eofhold
 DIM path: BYTE
 DIM rec(5): INTEGER
 DIM i, j, k: INTEGER
 FOR i = 1 TO 5
 rec(i) = 0
 NEXT i
 OPEN #path, "eof.dat": WRITE
 SEEK #path, 10
 rec(1) = 111
 PUT #path, rec
 PRINT #2, "A: PUT at EOF(10) done -- holding"
 FOR k = 1 TO 4
 FOR i = 1 TO 3000
 FOR j = 1 TO 3000
 NEXT j
 NEXT i
 NEXT k
 PRINT #2, "A: closing"
 CLOSE #path
 PRINT #2, "A: closed"
q
run eofhold
bye
