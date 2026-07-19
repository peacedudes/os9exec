PROCEDURE lockholder
! Dogfood 2026-07-19: the decisive record-lock test. Per the design
! (user-designed-rbf-eof-lock memory, restated by the original designer):
! a READ in update mode should LOCK the record it read, sleeping any
! other process that tries to GET the same record until this path's
! next WRITE releases it. This is the "discriminating question" used to
! establish that os9exec has NO record locking at all (source-level:
! SS_Lock is a no-op). This program is the "holder": GET the record,
! print a marker, burn real wall-clock time WITHOUT releasing the lock,
! print a second marker, THEN release (PUT) and close. lockwaiter.bas
! (launched concurrently) tries to GET the same record; if NitrOS-9 has
! a real lock, its "GET returned" marker should land AFTER this
! program's "about to release" marker, not before.
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
DIM i: INTEGER
OPEN #path, "/dd/CLAUDE/locktest.dat": UPDATE
SEEK #path, 0
GET #path, rec
PRINT #2, "holder: locked record, count="; rec.count; " -- holding"
FOR i = 1 TO 60000
NEXT i
PRINT #2, "holder: about to release (PUT)"
rec.count = rec.count + 1
SEEK #path, 0
PUT #path, rec
PRINT #2, "holder: released"
CLOSE #path
END
