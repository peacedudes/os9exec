! Sequential file I/O round-trip: CREATE, WRITE two lines, CLOSE, OPEN,
! READ both lines back, DELETE — confirmed on 68k. See
! basic09-fileio-eof-test.bas for the EOF() timing gotcha found while
! writing this test (EOF is still FALSE right after the last successful
! READ, only becoming TRUE after an over-read is attempted — so the
! check below intentionally expects FALSE, not TRUE). The
! DELETE-then-reopen check also incidentally confirms `ERR` auto-resets
! to 0 once read: the printed "ERR=0" is not a bug — reading ERR inside
! the `IF ERR<>0` check already reset it before PRINT re-read it.
!
! LIVE RESULT (68k): all 5 checks PASS
PROCEDURE t_fileio
DIM path: BYTE
DIM line: STRING[40]
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
CREATE #path, "fiotest": WRITE
WRITE #path, "first line"
WRITE #path, "second line"
CLOSE #path
OPEN #path, "fiotest": READ
total = total+1
IF EOF(#path)=FALSE THEN
 pass = pass+1
 PRINT "PASS: EOF false before reading anything"
ELSE
 fail = fail+1
 PRINT "FAIL: EOF true at start"
ENDIF
READ #path, line
total = total+1
IF line="first line" THEN
 pass = pass+1
 PRINT "PASS: sequential READ round-trips WRITE, got ["; line; "]"
ELSE
 fail = fail+1
 PRINT "FAIL: got ["; line; "]"
ENDIF
READ #path, line
total = total+1
IF line="second line" THEN
 pass = pass+1
 PRINT "PASS: second READ gets second line, got ["; line; "]"
ELSE
 fail = fail+1
 PRINT "FAIL: got ["; line; "]"
ENDIF
total = total+1
IF EOF(#path)=FALSE THEN
 pass = pass+1
 PRINT "PASS: EOF still false right after reading the last line (see basic09-fileio-eof-test.bas)"
ELSE
 fail = fail+1
 PRINT "FAIL: EOF true immediately after last successful READ"
ENDIF
CLOSE #path
DELETE "fiotest"
total = total+1
ON ERROR GOTO 900
OPEN #path, "fiotest": READ
PRINT "unreachable"
900 total = total
IF ERR<>0 THEN
 pass = pass+1
 PRINT "PASS: DELETE removed the file, reopen fails with ERR="; ERR
ELSE
 fail = fail+1
 PRINT "FAIL: reopen after DELETE succeeded"
ENDIF
ON ERROR
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
