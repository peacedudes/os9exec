PROCEDURE hammer
! Parameterized RBF hammer worker. ONE program, many scenarios.
!
! This is a TEMPLATE. The Swift driver substitutes the @TOKENS@ below and
! writes the result as a BASIC09 command script. Values are baked in rather
! than passed as command-line PARAMs on purpose:
!
!   - 6809 BASIC09 does NOT decimal-parse INTEGER PARAMs. It copies the
!     argument's raw ASCII bytes into the 2-byte INTEGER, so `hammer 42`
!     arrives as 13362 ($3432). STRING PARAMs work, but then every numeric
!     argument needs VAL() plus shell quoting that differs between the two
!     shells. Substitution sidesteps the whole class.
!
! Opened for UPDATE, never plain WRITE, and that is the point: RBF's
! automatic record locking applies to update-mode paths only. A plain
! write-only appender never makes a reader wait -- which is exactly why two
! programs appending to one log do not block each other.
!
! Record layout must match Record.swift byte for byte:
!   W<2> S<5> C<5> P<44 padded>  = 63 chars, + CR from PRINT = 64 bytes
! Fixed width is load-bearing: it makes a torn write distinguishable from a
! short one, and lets a reader verify a file still being appended to.
!
! The checksum is a plain byte sum. It needs no MOD here even though the
! format allows values to 65535: the payload is at most 44 chars of ASCII,
! so the sum cannot exceed 5588. That matters because 6809 BASIC09's INTEGER
! is SIGNED 16-bit -- the constant 65536 is not representable there at all,
! so a literal MOD 65536 would not even compile on half our targets.
!
! Roles: append, read
!
! The nap is a burst of writes to /nil. It is NOT a timed sleep: BASIC09
! cannot issue a SetStat, and F$Sleep would need an assembly Sbrtn built
! separately for each target. What it is instead is a burst of real system
! calls -- scheduling points -- on a device that is not the one under test,
! which is what actually lets another worker in. Whether that is enough to
! interleave with the tick OFF (-q) is an empirical question, measured per
! scenario, not an assumption: see FAILABILITY.md.
DIM path, nilpath: BYTE
DIM index, worker, seqnum, cksum, total, charpos: INTEGER
DIM napcount, napindex, gotcount: INTEGER
DIM payload: STRING[44]
DIM line: STRING[64]
DIM inline: STRING[64]
DIM pad: STRING[44]
DIM role: STRING[8]
DIM fname: STRING[64]
worker = @ID@
total = @COUNT@
napcount = @NAP@
role = "@ROLE@"
fname = "@FILE@"
pad = "                                            "
ON ERROR GOTO 900
OPEN #nilpath, "/nil": WRITE
IF role = "append" THEN 100
IF role = "read" THEN 500
PRINT #2, "hammer: FAIL unknown role "; role
END
!
! --- append: extend the file with self-describing records
100 CREATE #path, fname: UPDATE
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
  FOR napindex = 1 TO napcount
    PRINT #nilpath, "y"
  NEXT napindex
NEXT index
CLOSE #path
PRINT #2, "hammer: worker "; worker; " append done "; total
END
!
! --- read: walk the file back and count whole records
500 OPEN #path, fname: READ
gotcount = 0
510 READ #path, inline
gotcount = gotcount + 1
GOTO 510
!
900 IF ERR = 211 THEN 950
PRINT #2, "hammer: worker "; worker; " error "; ERR
END
950 CLOSE #path
PRINT #2, "hammer: worker "; worker; " read done "; gotcount
END
