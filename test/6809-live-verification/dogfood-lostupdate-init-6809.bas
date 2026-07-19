PROCEDURE lostinit
! Dogfood 2026-07-19: 6809 port of the 68k lost-update race test
! (test/68k-live-verification/dogfood-lostupdate-*.bas), verifying
! os9-systems-dev/file-managers.md's Record Locking "case 1" design
! intent on REAL NitrOS-9/6809 RBF, not just os9exec/68k: a
! database-style read-modify-write cycle should be race-safe under
! concurrent access with ZERO explicit application-level locking calls,
! because a Read in update mode automatically locks the record just
! read, and the following Write on that same path automatically
! releases it.
!
! Setup helper: creates the shared counter file fresh. count:INTEGER
! (2 bytes on 6809) plus four filler INTEGER fields push the count
! field's byte range away from the file's true EOF (10 bytes total),
! so every racer's WRITE lands mid-file, not at EOF -- keeping this on
! RBF's ordinary per-record lock path, not the separate whole-file
! EOF-lock mechanism (case 2, already tested separately).
!
! Note for 6809: comments must come AFTER the PROCEDURE line, not
! before -- a leading comment makes LOAD fail the whole file with
! Error #043 on this architecture (basic09/gotchas.md).
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM rec: rectype
CREATE #path, "/dd/CLAUDE/counter.dat": UPDATE
rec.count = 0
SEEK #path, 0
PUT #path, rec
CLOSE #path
PRINT #2, "lostinit: created counter.dat, count=0"
END
