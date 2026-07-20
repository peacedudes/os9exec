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
END

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
! Roles: append, read
DIM path: BYTE
DIM index, worker, seqnum, cksum, total, charpos: INTEGER
DIM napcount, gotcount, failed: INTEGER
DIM payload: STRING[44]
DIM line: STRING[64]
DIM inline: STRING[64]
DIM pad: STRING[44]
DIM role, napmode: STRING[8]
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
IF role = "append" THEN
  CREATE #path, fname: UPDATE
  FOR index = 0 TO total - 1
    seqnum = index
    payload = "w" + STR$(worker) + "-r" + STR$(seqnum)
    cksum = 0
    FOR charpos = 1 TO LEN(payload)
      cksum = cksum + ASC(MID$(payload, charpos, 1))
    NEXT charpos
    line = "W" + RIGHT$("00" + STR$(worker), 2)
    line = line + " S" + RIGHT$("00000" + STR$(seqnum), 5)
    line = line + " C" + RIGHT$("00000" + STR$(cksum), 5)
    line = line + " P" + LEFT$(payload + pad, 44)
    PRINT #path, line
    RUN hnap(napmode, napcount)
  NEXT index
  CLOSE #path
  PRINT #2, "hammer: worker "; worker; " append done "; total
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
    PRINT #2, "hammer: worker "; worker; " FAIL unknown role "; role
  ENDIF
ENDIF
END
