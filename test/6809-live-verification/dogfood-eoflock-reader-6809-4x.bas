PROCEDURE eofr4x
DIM path: BYTE
DIM line: STRING[128]
DIM n, retries, k, opentries: INTEGER
DIM t: STRING[8]
n = 0
retries = 0
opentries = 0
ON ERROR GOTO 20
10 OPEN #path, "/DD/EOFTEST/eof4x.dat": READ
GOTO 30
20 opentries = opentries + 1
IF opentries > 40 THEN 25
FOR k = 1 TO 300
NEXT k
GOTO 10
25 PRINT #2, "reader-RO: ABORT-OPEN opentries="; opentries; " err="; ERR; " time="; RIGHT$(DATE$,8)
GOTO 999
30 PRINT #2, "reader-RO: opened path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8); " opentries="; opentries
ON ERROR GOTO 800
100 READ #path, line
n = n + 1
PRINT #2, "reader-RO: read #"; n; " time="; RIGHT$(DATE$,8); " data="; LEFT$(line,15)
IF n < 200 THEN 100
GOTO 900
800 t = RIGHT$(DATE$,8)
IF ERR <> 211 THEN 950
retries = retries + 1
PRINT #2, "reader-RO: EOF retry #"; retries; " time="; t; " (n so far="; n; ")"
IF retries > 150 THEN 950
FOR k = 1 TO 1000
NEXT k
GOTO 100
900 PRINT #2, "reader-RO: SUCCESS all "; n; " records, retries="; retries; " time="; RIGHT$(DATE$,8)
GOTO 999
950 PRINT #2, "reader-RO: ABORT-READ n="; n; " err="; ERR; " retries="; retries; " time="; t
999 CLOSE #path
PRINT #2, "reader-RO: done"
END
