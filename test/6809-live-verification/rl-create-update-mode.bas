PROCEDURE lk1
 DIM path: BYTE
 DIM rec(5): INTEGER
 PRINT #2, "lk1: creating"
 CREATE #path, "lk.dat": UPDATE
 rec(1)=0
 PUT #path, rec
 CLOSE #path
 PRINT #2, "lk1: ok"
