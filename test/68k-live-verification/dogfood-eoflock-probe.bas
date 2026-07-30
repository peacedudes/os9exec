PROCEDURE eofprobe
! Captured 2026-07-30 from /dd/USR/CLAUDE/eofprobe.bas on the live 68k
! system disk, which is a working master not under version control -- this
! was the only copy. Run it before any timed dogfood-eoflock-* test: it
! exercises every BASIC09 feature those tests depend on with no timing and
! no second process, so a failure here is a syntax or path problem rather
! than a locking result.
! Smoke-test probe for dogfood-eoflock-* -- syntax sanity check only
! (DATE$/RIGHT$/STR$/string concat/CREATE.WRITE/OPEN.READ/WRITE#/READ#)
! before running the full timed EOF-lock test.
DIM path: BYTE
DIM t: STRING[8]
DIM line: STRING[24]
DIM back: STRING[24]
t = RIGHT$(DATE$,8)
line = t + " seq=" + STR$(7)
PRINT #2, "probe: built line="; line
CREATE #path, "/h1/CLAUDETEST/eofprobe.dat": WRITE
PRINT #2, "probe: created err="; ERR
WRITE #path, line
PRINT #2, "probe: wrote err="; ERR
CLOSE #path
OPEN #path, "/h1/CLAUDETEST/eofprobe.dat": READ
PRINT #2, "probe: opened for read err="; ERR
READ #path, back
PRINT #2, "probe: read back="; back; " err="; ERR
CLOSE #path
PRINT #2, "probe: done"
END
