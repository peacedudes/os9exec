PROCEDURE b8test
 DIM p:INTEGER
 DIM s:STRING[40]
 CREATE #p,"/DD/b8out":WRITE
 PRINT #p USING "B8",TRUE
 PRINT #p USING "B8",FALSE
 CLOSE #p
 OPEN #p,"/DD/b8out":READ
 READ #p,s
 PRINT "T[";s;"] len=";LEN(s)
 READ #p,s
 PRINT "F[";s;"] len=";LEN(s)
 CLOSE #p
 END
