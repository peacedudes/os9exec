PROCEDURE eofw4x
DIM path: BYTE
DIM i, j, plen, pausesecs: INTEGER
DIM t, holdt: STRING[8]
DIM padfull: STRING[120]
DIM prefix: STRING[40]
DIM line: STRING[128]
CREATE #path, "/DD/EOFTEST/eof4x.dat": WRITE
PRINT #2, "writer: created path="; path; " err="; ERR; " time="; RIGHT$(DATE$,8)
padfull = ""
FOR j = 1 TO 120
padfull = padfull + "X"
NEXT j
FOR i = 1 TO 200
prefix = "seq=" + STR$(i) + " t=" + RIGHT$(DATE$,8) + " pad="
plen = LEN(prefix)
line = prefix + LEFT$(padfull, 128 - plen)
WRITE #path, line
NEXT i
PRINT #2, "writer: wrote 200 x128B records fast, holding open, time="; RIGHT$(DATE$,8)
holdt = RIGHT$(DATE$,8)
pausesecs = 0
200 t = RIGHT$(DATE$,8)
IF t = holdt THEN 200
holdt = t
pausesecs = pausesecs + 1
IF pausesecs < 2 THEN 200
CLOSE #path
PRINT #2, "writer: closed at "; RIGHT$(DATE$,8); " -- done, 200 records, held open ~2s"
END
