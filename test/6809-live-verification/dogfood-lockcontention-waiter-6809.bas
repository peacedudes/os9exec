PROCEDURE lockwaiter
! Dogfood 2026-07-19: companion to lockholder.bas -- see that file's
! header for the full design. This is the "waiter": open the same file
! (also in UPDATE mode, so its own GET would itself try to acquire the
! lock), print a marker, then GET the same record the holder locked. If
! NitrOS-9 has a real record lock, this GET should not return until the
! holder's PUT releases it -- the "GET returned" marker should land
! AFTER the holder's "about to release" marker in the interleaved
! output. If there is no real lock, "GET returned" lands almost
! immediately, well before the holder's release.
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
OPEN #path, "/dd/CLAUDE/locktest.dat": UPDATE
PRINT #2, "waiter: about to GET"
SEEK #path, 0
GET #path, rec
PRINT #2, "waiter: GET returned, count="; rec.count
CLOSE #path
END
