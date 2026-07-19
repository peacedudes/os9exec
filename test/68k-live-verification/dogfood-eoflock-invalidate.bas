PROCEDURE eofinv
! Cache-invalidation case, and the reason a "reader sees the write" test is
! not enough on its own. Here the reader touches the data FIRST, so it is
! holding that sector in its own buffer before the writer changes it. A path
! only re-reads a sector when the one it holds is a different one, so without
! the writer reaching into the other paths and dropping the sector they hold,
! this reader goes on serving itself the copy it cached and never sees the
! second record at all.
!
! One process, two paths, no concurrency and no timing -- the staleness is
! structural, not a race, so it reproduces every single time.
!
! Sequence: write A, read A (reader now caches the sector), write B, read B.
! Expect "OK second=BBB". A stale reader gets end-of-file or a repeat of AAA.
DIM w, r: BYTE
DIM line: STRING[20]
ON ERROR GOTO 800
CREATE #w, "inv.dat": WRITE
OPEN #r, "inv.dat": READ
WRITE #w, "AAA"
READ #r, line
PRINT "eofinv: first="; line
WRITE #w, "BBB"
READ #r, line
PRINT "eofinv: OK second="; line
GOTO 900
800 ON ERROR GOTO 0
PRINT "eofinv: FAIL err="; ERR
900 END
