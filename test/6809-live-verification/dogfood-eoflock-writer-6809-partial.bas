PROCEDURE eofwrpt
DIM path: BYTE
DIM i, j, plen, pausesecs: INTEGER
DIM t, holdt: STRING[8]
DIM padfull: STRING[60]
DIM prefix: STRING[40]
DIM line: STRING[80]
DIM crb: BYTE
CREATE #path, "/DD/EOFTEST/eofput.dat": WRITE
PRINT #2, "writer: created path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
crb = 13
padfull = ""
FOR j = 1 TO 60
padfull = padfull + "X"
NEXT j
FOR i = 1 TO 50
prefix = "seq=" + STR$(i) + " t=" + RIGHT$(DATE$,8) + " pad="
plen = LEN(prefix)
line = prefix + LEFT$(padfull, 80 - plen)
WRITE #path, line
PUT #path, crb
NEXT i
PRINT #2, "writer: wrote 50 records+PUT fast, holding open, time="; RIGHT$(DATE$,8)
holdt = RIGHT$(DATE$,8)
pausesecs = 0
200 t = RIGHT$(DATE$,8)
IF t = holdt THEN 200
holdt = t
pausesecs = pausesecs + 1
IF pausesecs < 10 THEN 200
CLOSE #path
PRINT #2, "writer: closed at "; RIGHT$(DATE$,8); " -- done, 50 records+PUT, held open ~10s"
END
