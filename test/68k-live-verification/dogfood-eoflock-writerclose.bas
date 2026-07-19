PROCEDURE eofwcl
! The writer leaves while a reader is still open -- the lifecycle case.
!
! While both are open the writer is the one whose view of the file is
! current: it knows the size the file has grown to and the segments it grew
! into. When it closes, that goes away. Anything that made the reader depend
! on the writer still being there -- a shared buffer, or a lookup that finds
! "whoever is writing" -- leaves the reader falling back to what it knew when
! it opened, which is nothing. The reader would have been correct right up
! until the writer left, and then silently wrong, which is worse than being
! wrong from the start because a short test never catches it.
!
! Note also that close is not a no-op for the file's shape: it trims unused
! tail sectors and rewrites the segment list, so what the reader must end up
! with is the state AFTER close, not a snapshot taken before it.
!
! Sequence: write two records, close the writer, then read both through the
! path that was open the whole time. Expect both records and then a clean
! end-of-file (211) -- not a short read, and not data that vanished.
DIM w, r: BYTE
DIM line: STRING[20]
DIM n: INTEGER
n = 0
ON ERROR GOTO 800
CREATE #w, "wcl.dat": WRITE
OPEN #r, "wcl.dat": READ
WRITE #w, "ONE"
WRITE #w, "TWO"
CLOSE #w
READ #r, line
n = n + 1
PRINT "eofwcl: rec1="; line
READ #r, line
n = n + 1
PRINT "eofwcl: rec2="; line
READ #r, line
PRINT "eofwcl: FAIL read a third record ="; line
GOTO 900
800 ON ERROR GOTO 0
IF ERR = 211 AND n = 2 THEN 850
PRINT "eofwcl: FAIL err="; ERR; " after n="; n
GOTO 900
850 PRINT "eofwcl: OK both records then clean EOF"
900 END
