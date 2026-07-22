PROCEDURE hnap
! Yield the CPU so another worker can get in between our read and our write.
!
! Two mechanisms, chosen by the caller:
!
!   "sleep" : SHELL "sleep <ticks>" -- a genuine F$Sleep block, not a busy
!             wait. `sleep` counts TICKS by default (`-s` switches it to
!             seconds), so this is ~10ms granularity at the standard 100Hz
!             tick: fine enough to open a real window between a read and its
!             write. Costs a process fork per nap.
!             NEVER pass 0. Zero means "sleep until signalled" -- forever,
!             here -- and it wedges the run. The caller clamps; this guards
!             again anyway.
!             68k: verified live. 6809: the SHELL statement is documented
!             only in the 68k-era manual, and its absence on 6809 is
!             UNCONFIRMED rather than established. Verify before trusting it
!             there; fall back to "nil" if it is missing.
!
!   "nil"   : a burst of writes to /nil. Not timed and not a block, just real
!             system calls -- scheduling points -- aimed at a device that is
!             not the one under test. Pure BASIC09, so it works wherever
!             BASIC09 does. This is the portable floor.
!
! Whether either is enough to interleave with the tick OFF (-q) is measured
! per scenario, never assumed. See FAILABILITY.md.
PARAM mode: STRING[8]
PARAM ticks: INTEGER
DIM nilpath: BYTE
DIM spin: INTEGER
ON ERROR GOTO 800
IF ticks > 0 THEN
  IF mode = "sleep" THEN
    SHELL "sleep " + STR$(ticks)
  ELSE
    OPEN #nilpath, "/nil": WRITE
    FOR spin = 1 TO ticks
      PRINT #nilpath, "y"
    NEXT spin
    CLOSE #nilpath
  ENDIF
ENDIF
GOTO 810
800 PRINT #2, "hammer: hnap ERROR "; ERR
810 END

PROCEDURE hwork
! Parameterized RBF hammer worker. ONE program, many scenarios.
!
! This is a TEMPLATE. The Swift driver substitutes the @TOKENS@ and writes the
! result as a BASIC09 command script. Values are baked in rather than passed as
! command-line PARAMs on purpose: 6809 BASIC09 does NOT decimal-parse INTEGER
! command-line PARAMs -- it copies the argument's raw ASCII bytes into the
! 2-byte INTEGER, so `hwork 42` arrives as 13362 ($3432). Substitution sidesteps
! the whole class. (PARAMs on an internal RUN, as hnap uses above, are fine --
! the defect is specific to command-line binding.)
!
! Opened for UPDATE, never plain WRITE, and that is the point: RBF's automatic
! record locking applies to update-mode paths only. A plain write-only appender
! never makes a reader wait -- which is exactly why two programs appending to
! one log do not block each other.
!
! Record layout must match Record.swift byte for byte:
!   W<2> S<5> C<5> P<44 padded>  = 63 chars, + CR from PRINT = 64 bytes
! Fixed width is load-bearing: it makes a torn write distinguishable from a
! short one, and lets a reader verify a file that is still being appended to.
!
! The checksum is a plain byte sum and needs no MOD here, even though the
! format allows values to 65535: the payload is at most 44 ASCII characters, so
! the sum cannot exceed 5588. That matters because 6809 BASIC09's INTEGER is
! SIGNED 16-bit -- the constant 65536 is not representable there at all, so a
! literal MOD 65536 would not even compile on half our targets.
!
! Roles:
!   append  CREATEs its own file and extends it. One worker per file.
!   slot    OPENs a file SEVERAL workers share, and writes only the byte range
!           it owns: records [slotbase .. slotbase+total), where
!           slotbase = (id-1)*total.
!           NOTE `slotbase`, not `base`: BASE is a RESERVED WORD in BASIC09
!           (the BASE 0 / BASE 1 array-origin statement), and using it as a
!           variable is rejected at entry with Error #000:012 -- the same trap
!           that `sq` fell into. Short names collide here surprisingly often.
!           Disjoint ranges are deliberate. BASIC09 has no seek-to-end, so
!           racing appenders would all write at offset 0 and simply overwrite
!           each other -- proving nothing about locking and everything about
!           the test being wrong. With computed slots every worker's bytes are
!           its own, so ANY cross-talk is corruption rather than an expected
!           collision, and the FD ring, sector buffers and segment list are all
!           exercised concurrently on ONE file.
!   read    walks a file back and counts whole records.
!   rmw     read-modify-writes ONE shared record, overlapping every other
!           rmw worker. The only role that can detect a missing record lock.
!   create  makes an empty file and exits. Used to provision a shared file
!           before the racers start, since two workers both CREATEing one file
!           is itself an error and would mask the results.
DIM path, wpath: BYTE
DIM index, worker, seqnum, cksum, total, charpos: INTEGER
! The binary tally record for the rmwbin/seedbin roles: five 16-bit integers
! = 10 bytes, the exact shape of the proven lost-update reproduction. GET/PUT
! move the whole 10 bytes; the tally lives in rec(1). Small (<256 bytes) on
! purpose -- the binary lock-retry bug re-presents a clobbered byte count and
! only bites requests that fit in the low byte.
DIM rec(5): INTEGER
! A LARGE binary record for the rmwbig/seedbig roles: 200 x 16-bit = 400 bytes,
! which SPANS MULTIPLE 256-byte sectors. The 6809 lost-update bug bit requests
! that fit in one sector (<256 bytes); a multi-sector GET/PUT under the same
! contention is a distinct read/lock path worth hammering separately. Tally in
! recbig(1), same as the small record, so the host parser is unchanged.
DIM recbig(200): INTEGER
DIM napcount, gotcount, failed, slotbase, slotnum, tally: INTEGER
DIM payload: STRING[44]
DIM line: STRING[64]
DIM inline: STRING[64]
DIM pad: STRING[44]
DIM napmode: STRING[8]
! role must hold the longest role name ("writeonly" = 9); STRING[8] silently
! TRUNCATES it to "writeonl" so the dispatch falls through to "unknown role".
DIM role: STRING[16]
DIM fname: STRING[64]
worker = @ID@
total = @COUNT@
napcount = @NAP@
role = "@ROLE@"
napmode = "@NAPMODE@"
fname = "@FILE@"
pad = "                                            "
gotcount = 0
failed = 0
ON ERROR GOTO 900
IF role = "seed" THEN
  ! One record holding a decimal tally, for the rmw role to increment.
  CREATE #path, fname: UPDATE
  PRINT #path, "T00000000" + LEFT$(pad + pad, 54)
  CLOSE #path
  PRINT #2, "hammer: seeded "; fname
ELSE
IF role = "create" THEN
  ! PRE-EXTEND the file to its full final size. This is not tidiness, it is
  ! required: OS-9 does NOT support writing into a hole. SEEKing past the
  ! current end of file and writing fails with E$EOF (211) -- a slot worker
  ! whose range starts beyond the current end simply dies, silently as far as
  ! the shell is concerned.
  !
  ! That defect cost real time and looked exactly like an RBF data-loss bug:
  ! one worker's entire range read back as zeros while the worker reported
  ! success, intermittently, only on shared files. It was this, every time.
  !
  ! The filler is deliberately NOT a valid record. An unwritten slot must read
  ! back as a torn record so a worker that failed to write is DETECTED, rather
  ! than being papered over by plausible-looking filler.
  CREATE #path, fname: UPDATE
  FOR index = 0 TO total - 1
    PRINT #path, LEFT$("---------- unwritten slot " + pad + pad, 63)
  NEXT index
  CLOSE #path
  PRINT #2, "hammer: pre-extended "; fname; " to "; total; " slots"
ELSE
IF role = "append" OR role = "slot" THEN
  IF role = "slot" THEN
    OPEN #path, fname: UPDATE
    slotbase = (worker - 1) * total
  ELSE
    CREATE #path, fname: UPDATE
    slotbase = 0
  ENDIF
  FOR index = 0 TO total - 1
    slotnum = slotbase + index
    seqnum = slotnum
    payload = "w" + STR$(worker) + "-r" + STR$(seqnum)
    cksum = 0
    FOR charpos = 1 TO LEN(payload)
      cksum = cksum + ASC(MID$(payload, charpos, 1))
    NEXT charpos
    line = "W" + RIGHT$("00" + STR$(worker), 2)
    line = line + " S" + RIGHT$("00000" + STR$(seqnum), 5)
    line = line + " C" + RIGHT$("00000" + STR$(cksum), 5)
    line = line + " P" + LEFT$(payload + pad, 44)
    IF role = "slot" THEN
      SEEK #path, slotnum * 64
    ENDIF
    PRINT #path, line
    RUN hnap(napmode, napcount)
  NEXT index
  CLOSE #path
  PRINT #2, "hammer: worker "; worker; " wrote "; total
ELSE
  IF role = "read" THEN
    ! The one numbered line in this program. BASIC09's EOF() is sticky like
    ! C's feof(): it only becomes TRUE after a read has already FAILED, so
    ! `WHILE NOT EOF(#path)` always over-reads once and raises E$EOF (211).
    ! Catching that error is the documented way to end the loop cleanly --
    ! there is no file-size function to loop a computed count against.
    ON ERROR GOTO 100
    OPEN #path, fname: READ
    LOOP
      READ #path, inline
      gotcount = gotcount + 1
      RUN hnap(napmode, napcount)
    ENDLOOP
100 failed = ERR
    IF failed = 211 THEN
      CLOSE #path
      PRINT #2, "hammer: worker "; worker; " read done "; gotcount
    ELSE
      PRINT #2, "hammer: worker "; worker; " FAIL error "; failed
    ENDIF
  ELSE
  IF role = "rmw" THEN
    ! THE RECORD-LOCK TEST. Every worker read-modify-writes the SAME record,
    ! so the extents OVERLAP -- which is the only way LockHolder ever reports a
    ! conflict. The `slot` roles cannot detect a missing lock at all, because
    ! disjoint ranges never overlap.
    !
    ! UPDATE mode is essential: RBF auto-locks the record a read returns, and
    ! the following write on that same path releases it. No SS_Lock call is
    ! made anywhere here -- BASIC09 cannot issue a SetStat, and the automatic
    ! lock is what is under test.
    !
    ! The nap sits BETWEEN the read and the write, holding the lock open. That
    ! is the window a competing worker must be kept out of; without it the
    ! read-modify-write completes too fast to ever interleave, which is exactly
    ! why the old counter race passed 600/600 against code with no locking.
    OPEN #path, fname: UPDATE
    FOR index = 1 TO total
      SEEK #path, 0
      READ #path, inline
      tally = VAL(MID$(inline, 2, 8)) + 1
      RUN hnap(napmode, napcount)
      SEEK #path, 0
      PRINT #path, "T" + RIGHT$("00000000" + STR$(tally), 8) + LEFT$(pad + pad, 54)
    NEXT index
    CLOSE #path
    PRINT #2, "hammer: worker "; worker; " rmw done "; total
  ELSE
  IF role = "rmwfree" THEN
    ! CONTROL for the rmw role: the SAME read-modify-write, deliberately
    ! WITHOUT the automatic record lock.
    !
    ! RBF auto-locks only on UPDATE-mode paths. Reading through a READ path and
    ! writing through a separate WRITE path therefore takes no lock at all --
    ! an unlocked RMW, with no change to the emulator whatsoever.
    !
    ! If `rmw` (locked) keeps a correct tally while this loses updates, the lock
    ! is demonstrably doing the work. If BOTH stay correct, the scenario never
    ! interleaves and neither result means anything.
    OPEN #path, fname: READ
    OPEN #wpath, fname: WRITE
    FOR index = 1 TO total
      SEEK #path, 0
      READ #path, inline
      tally = VAL(MID$(inline, 2, 8)) + 1
      RUN hnap(napmode, napcount)
      SEEK #wpath, 0
      PRINT #wpath, "T" + RIGHT$("00000000" + STR$(tally), 8) + LEFT$(pad + pad, 54)
    NEXT index
    CLOSE #path
    CLOSE #wpath
    PRINT #2, "hammer: worker "; worker; " rmwfree done "; total
  ELSE
  IF role = "rmwbin" THEN
    ! THE LIVE LOST-UPDATE REPRODUCTION. Identical loop to `rmw`, but BINARY
    ! GET/PUT on the 10-byte integer record instead of text READ/PRINT. The
    ! text path's lock serializes correctly (rmw keeps a perfect tally); this
    ! binary path is the one whose lock-retry-after-park loses updates on stock
    ! NitrOS-9. nap=0 (back-to-back) so the racers genuinely park and wake on
    ! each other, which is what triggers the clobbered-count bug.
    OPEN #path, fname: UPDATE
    FOR index = 1 TO total
      SEEK #path, 0
      GET #path, rec
      rec(1) = rec(1) + 1
      RUN hnap(napmode, napcount)
      SEEK #path, 0
      PUT #path, rec
    NEXT index
    CLOSE #path
    PRINT #2, "hammer: worker "; worker; " rmwbin done "; total
  ELSE
  IF role = "seedbin" THEN
    ! Provision the binary tally file: one 10-byte record, all zeros.
    CREATE #path, fname: UPDATE
    FOR index = 1 TO 5
      rec(index) = 0
    NEXT index
    PUT #path, rec
    CLOSE #path
    PRINT #2, "hammer: seeded "; fname
  ELSE
  IF role = "rmwbig" THEN
    ! Same lost-update loop as rmwbin, but a 400-byte MULTI-SECTOR record. The
    ! 6809 fix was proven on single-sector (<256 byte) requests; a GET/PUT that
    ! spans sectors is a distinct read/lock path and may not be covered.
    OPEN #path, fname: UPDATE
    FOR index = 1 TO total
      SEEK #path, 0
      GET #path, recbig
      recbig(1) = recbig(1) + 1
      RUN hnap(napmode, napcount)
      SEEK #path, 0
      PUT #path, recbig
    NEXT index
    CLOSE #path
    PRINT #2, "hammer: worker "; worker; " rmwbig done "; total
  ELSE
  IF role = "seedbig" THEN
    ! Provision the 400-byte tally record, all zeros.
    CREATE #path, fname: UPDATE
    FOR index = 1 TO 200
      recbig(index) = 0
    NEXT index
    PUT #path, recbig
    CLOSE #path
    PRINT #2, "hammer: seeded "; fname
  ELSE
  IF role = "writeonly" THEN
    ! Write-ONLY producer. CREATE the file in WRITE mode (hits the Creat lock
    ! gate) and write `total` records: the first two fast, the rest each after a
    ! nap, so a concurrent follower reaches EOF at two records. A write-only
    ! producer must take NO record/eof lock (owner's design: writers create no
    ! locks); stock NitrOS-9 wrongly does, and the follower then trails it.
    CREATE #path, fname: WRITE
    FOR index = 1 TO total
      line = "W" + RIGHT$("00" + STR$(worker), 2)
      line = line + " R" + RIGHT$("00000" + STR$(index), 5) + LEFT$(pad + pad, 54)
      PRINT #path, line
      IF index >= 2 THEN
        RUN hnap(napmode, napcount)
      ENDIF
    NEXT index
    CLOSE #path
    PRINT #2, "hammer: worker "; worker; " wrote "; total
  ELSE
  IF role = "follow" THEN
    ! Follower for the write-only producer. Nap briefly so the producer creates
    ! the file first, then read to EOF counting records. On stock the producer's
    ! bogus eof lock makes this BLOCK at EOF and FOLLOW (count == producer total);
    ! with the fix it stops at the initial EOF (count small). The count is the
    ! whole signal.
    RUN hnap(napmode, napcount)
    ON ERROR GOTO 120
    OPEN #path, fname: READ
    LOOP
      READ #path, inline
      gotcount = gotcount + 1
    ENDLOOP
120 failed = ERR
    IF failed = 211 THEN
      CLOSE #path
      PRINT #2, "hammer: worker "; worker; " follow done "; gotcount
    ELSE
      PRINT #2, "hammer: worker "; worker; " FAIL error "; failed
    ENDIF
  ELSE
    PRINT #2, "hammer: worker "; worker; " FAIL unknown role "; role
  ENDIF
  ENDIF
  ENDIF
  ENDIF
  ENDIF
  ENDIF
  ENDIF
  ENDIF
  ENDIF
ENDIF
ENDIF
ENDIF
GOTO 910
900 PRINT #2, "hammer: worker "; worker; " ERROR err "; ERR
910 END
