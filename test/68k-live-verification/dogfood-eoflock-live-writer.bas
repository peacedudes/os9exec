PROCEDURE eofwr2
! Opened for UPDATE, not WRITE, and that is deliberate: locking belongs to
! update-mode opens only. A plain write-only appender never makes a reader
! wait -- which is what stops two programs appending to one log from ever
! getting in each other's way. A writer that wants a reader to be able to
! follow it says so by opening for update.
DIM p: BYTE
DIM i: INTEGER
DIM t, lastt: STRING[8]
DIM line: STRING[24]
CREATE #p, "/h1/CLAUDETEST/live.dat": UPDATE
lastt = RIGHT$(DATE$,8)
i = 0
100 t = RIGHT$(DATE$,8)
IF t = lastt THEN 100
lastt = t
i = i + 1
line = t
WRITE #p, line
PRINT #2, "W"; i; " wrote at "; t
IF i < 6 THEN 100
CLOSE #p
PRINT #2, "W done"
END
