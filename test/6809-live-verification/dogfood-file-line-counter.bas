! Dogfood task 2026-07-18: an ordinary BASIC09 program, not tied to a
! specific skill claim. Prompts for a filename, creates it and writes
! user-entered lines to it (sentinel: a blank line ends input), closes
! it, reopens it for reading, reads it back line by line, and reports
! total lines and total characters (excluding the CR line terminator
! each WRITE/READ adds/strips automatically).
!
! Entered via BASIC09's interactive line editor (`e linecnt` from the
! B: prompt, one line at a time via nitros9repl.sh's `key` interface --
! `send` doesn't recognize the E: sub-prompt, see
! 6809/using-nitros9-repl.md), then run live on real NitrOS-9 (6809)
! BASIC09 "6809 VERSION 01.01.00" under XRoar via tools/nitros9repl.sh.
!
! Test input (filename "lctest1"):
!   Hello from BASIC09           (18 chars)
!   OS-9 file test line two      (23 chars)
!   Third line here               (15 chars)
!   <blank line -- ends input>
!
! Literal program output:
!   File:           lctest1
!   Lines read:     3
!   Characters (excl. terminators): 56
!
! Independently cross-checked at the OS-9 shell (not just trusting the
! program's own count):
!   fsize lctest1  ->  59 Bytes in 1 Files   (56 chars + 3 CR terminators = 59, matches)
!   list lctest1   ->  the 3 lines back out exactly as typed
!
! Gotcha noted along the way (cosmetic, doesn't affect this program's
! correctness): a literal semicolon typed inside a PRINT string literal
! -- "Enter lines of text; blank line ends input:" -- comes back with a
! backslash escape both in LIST's source listing AND in the actual
! PRINT output at runtime ("text\; blank line..."). Not a bug in this
! program, just a real quirk of how BASIC09 stores/echoes a semicolon
! (normally a statement separator) inside a string constant.

PROCEDURE linecnt
DIM fname: STRING[32]
DIM line$: STRING[80]
DIM wp, rp: BYTE
DIM linecount, charcount: INTEGER

INPUT "Filename to create: ", fname
CREATE #wp, fname: WRITE
PRINT "Enter lines of text; blank line ends input:"
INPUT "> ", line$
WHILE LEN(line$) > 0 DO
  WRITE #wp, line$
  INPUT "> ", line$
ENDWHILE
CLOSE #wp

OPEN #rp, fname: READ
linecount = 0
charcount = 0
ON ERROR GOTO 900
LOOP
  READ #rp, line$
  linecount = linecount + 1
  charcount = charcount + LEN(line$)
ENDLOOP
900 CLOSE #rp

PRINT
PRINT "File:", fname
PRINT "Lines read:", linecount
PRINT "Characters (excl. terminators):", charcount
END
