! EOF() timing gotcha — confirmed on 68k: EOF(#path) does NOT become TRUE
! right after reading the last record in a sequential file. It stays
! FALSE until a further READ is actually attempted past the end, which
! raises `Error #000:211 (E$EOF)` (a standard OS-9 system-level I/O error
! code, not a BASIC09-specific one — see common/error-codes.md). This
! means the natural-looking loop idiom `WHILE NOT EOF(#path) DO
! READ #path,x ... ENDWHILE` always attempts one over-read on its final
! iteration and needs `ON ERROR GOTO` to catch it cleanly.
!
! LIVE RESULT (68k): both checks PASS, ERR=211 confirmed on the over-read
PROCEDURE t_fileio2
DIM path: BYTE
DIM line: STRING[40]
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
CREATE #path, "fiotest2": WRITE
WRITE #path, "first line"
WRITE #path, "second line"
CLOSE #path
OPEN #path, "fiotest2": READ
READ #path, line
READ #path, line
total = total+1
IF EOF(#path)=FALSE THEN
 pass = pass+1
 PRINT "PASS (confirms): EOF still false right after reading the LAST record"
ELSE
 fail = fail+1
 PRINT "FAIL: EOF true immediately after last successful READ"
ENDIF
ON ERROR GOTO 100
READ #path, line
PRINT "unreachable — read past end should trap"
100 total = total+1
IF EOF(#path)=TRUE THEN
 pass = pass+1
 PRINT "PASS: EOF becomes true only AFTER an attempted over-read, ERR="; ERR
ELSE
 fail = fail+1
 PRINT "FAIL: EOF still false even after over-read attempt"
ENDIF
ON ERROR
CLOSE #path
DELETE "fiotest2"
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
