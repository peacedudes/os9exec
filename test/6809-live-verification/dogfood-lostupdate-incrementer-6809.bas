PROCEDURE lostinc
! Dogfood 2026-07-19: 6809 port of the lost-update racer, see
! lostinit.bas's header and dogfood-report-lostupdate-2026-07-18.md (68k
! original) for the full design. No SS_Lock anywhere in this program --
! that's the whole point. Opens the shared counter file for UPDATE once,
! loops N=300 times doing SEEK 0 / GET / +1 / SEEK 0 / PUT, path held
! open across all iterations. Two instances of this PACKed module race
! concurrently, launched as a single combined shell command line
! (`lostinc & lostinc &`) to avoid the documented "second separate key
! send is unreliable" launch-ordering gotcha.
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM i, iters: INTEGER
DIM rec: rectype
iters = 300
OPEN #path, "/dd/CLAUDE/counter.dat": UPDATE
FOR i = 1 TO iters
  SEEK #path, 0
  GET #path, rec
  rec.count = rec.count + 1
  SEEK #path, 0
  PUT #path, rec
NEXT i
CLOSE #path
PRINT #2, "lostinc: done, "; iters; " iterations, last local count seen="; rec.count
END
