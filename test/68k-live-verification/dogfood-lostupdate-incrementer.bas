PROCEDURE lostinc
! Dogfood task 2026-07-18: lost-update race, RBF automatic record-locking
! design intent (os9-systems-dev's file-managers.md "Record Locking",
! case 1, firsthand design intent from the original Microware designer,
! per user memory): a read-modify-write cycle should be race-safe under
! concurrent access with ZERO explicit application-level locking calls
! (no SS_Lock anywhere in this program), because a Read in update mode
! automatically locks the record just read, and the following Write on
! that same path automatically releases it.
!
! This racer: opens the shared counter file (created by
! dogfood-lostupdate-init.bas) for UPDATE once, then loops N=300 times
! doing SEEK 0 / GET (read) / +1 / SEEK 0 / PUT (write), keeping the
! path open across all iterations rather than re-opening per iteration
! -- the natural "long-lived connection" shape for exercising the
! per-path lock/unlock pair repeatedly; a fresh open/close per iteration
! would also work (each iteration's Read-then-Write still shares one
! path) but adds nothing here and is slower.
!
! Two instances of this exact PACKed module race concurrently against
! the same file, launched from the OS-9 shell as a single combined
! command line (`lostinc & lostinc &`) per
! using-os9exec-repl.md's "Launching two concurrent background
! processes reliably" section -- a second separate `key` send to launch
! the trailing process was found unreliable elsewhere in this project's
! history (dropped, delayed, or interleaved with the first process's
! output); one combined line avoids that class of failure entirely.
!
! If RBF's automatic locking works as intended, the file's final count
! equals 2*N exactly, no lost updates, even though neither instance ever
! calls SS_Lock. See dogfood-lostupdate-verify.bas for the final-count
! check and dogfood-report-lostupdate-2026-07-18.md for the full
! narrative, including the "math" trap-handler prerequisite gotcha
! (load math before running any of this trio) and PACK/RunB workflow
! notes.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh -r,
! account "claude", chx=/dd/CMDS; PACKed to /dd/CMDS/lostinc, launched
! bare-name; two independent full races, both starting from count=0):
!
!   Race 1:
!     claude: lostinc & lostinc &
!     +6
!     lostinc: done, 300 iterations, last local count seen=300
!     +7
!     claude: lostinc: done, 300 iterations, last local count seen=600
!   (dogfood-lostupdate-verify.bas afterward: "lostver: final count=600")
!
!   Race 2 (counter reset to 0 via dogfood-lostupdate-init.bas between
!   races):
!     claude: lostinc & lostinc & procs
!     +6
!     lostinc: done, 300 iterations, last local count seen=300
!     +7
!      Id PId Grp.Usr  Prior  MemSiz Sig S    CPU Time   Age Module & I/O
!     lostinc: done, 300 iterations, last local count seen=600
!   (dogfood-lostupdate-verify.bas afterward: "lostver: final count=600")
!
! Both independent races landed exactly on 2*N=600 -- no lost updates in
! either run. (The `procs` snapshot in race 2, sent immediately after
! backgrounding both, came back empty of lostinc processes -- both
! racers completed within roughly 0.1-0.4 real seconds, too fast for
! this particular process-table probe to catch them mid-flight; see the
! report for what that does and doesn't prove about genuine
! interleaving.)
TYPE rectype = count: INTEGER; f1: INTEGER; f2: INTEGER; f3: INTEGER; f4: INTEGER
DIM path: BYTE
DIM i, iters: INTEGER
DIM rec: rectype
iters = 300
OPEN #path, "/h1/CLAUDETEST/counter.dat": UPDATE
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
