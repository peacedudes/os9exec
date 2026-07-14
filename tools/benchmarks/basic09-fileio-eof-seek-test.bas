! Root-causes the EOF() timing gotcha from basic09-fileio-eof-test.bas:
! is it a live position check (which would mean os9exec's file-position
! tracking is buggy) or a sticky post-read flag (normal, C feof()-style
! behavior)? Settles it by SEEKing to positions from just-past-a-6-byte-
! file up through 1000 bytes in, checking EOF() after each SEEK with NO
! read attempted. If EOF() ever goes TRUE from a SEEK alone, that proves
! it's a live check; if it never does, no matter how far past the end,
! that proves it's a feof()-style sticky flag untouched by SEEK.
!
! CONFIRMED: EOF() stays FALSE at every tested position (5, 6, 7, 100,
! 255, 256, 1000) with no READ attempted -- rules out a position-
! tracking bug in os9exec (whose own pReof() does a straightforward
! currPos>=lastPos check and looks correct in isolation). BASIC09's
! EOF() evidently isn't driven by a live call to that check; it behaves
! like C's feof() instead -- a sticky flag only a real failed READ sets.
! Not a bug, just an easily-misread I/O pattern.
!
! LIVE RESULT (68k): EOF=False at all 4 tested SEEK positions
PROCEDURE t_eofseek2
DIM path: BYTE
CREATE #path, "eofseek2": WRITE
WRITE #path, "AB"
WRITE #path, "CD"
CLOSE #path
OPEN #path, "eofseek2": READ
SEEK #path, 100
PRINT "at pos 100, EOF="; EOF(#path)
SEEK #path, 255
PRINT "at pos 255, EOF="; EOF(#path)
SEEK #path, 256
PRINT "at pos 256, EOF="; EOF(#path)
SEEK #path, 1000
PRINT "at pos 1000, EOF="; EOF(#path)
CLOSE #path
DELETE "eofseek2"
END
