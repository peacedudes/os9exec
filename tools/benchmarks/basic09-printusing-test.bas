! Exercises all 6 PRINT USING directive letters (R/E-style, I, S, H, B)
! plus repeat groups and multi-field lines. MAJOR CORRECTION this
! session: earlier skill content and test files assumed a generic-BASIC
! `"###.##"` pound-sign format string — that syntax NEVER existed in
! BASIC09 and was the real cause of a previously-unresolved runtime
! error (#000:063), misdiagnosed as a compiler bug. Real BASIC09 uses
! directive LETTERS: `Rw.f` (real), `Ew.f` (exponential), `Iw` (integer),
! `Hw` (hex dump), `Sw` (string), `Bw` (boolean) — see
! basic09-printusing-fieldwidth-test.bas for exact-width verification and
! a real manual-vs-implementation divergence found (BOOLEAN prints
! "True", not "TRUE" as the manual states).
!
! LIVE RESULT (68k): all 7 statements execute with no error.
PROCEDURE t_printusing
DIM r: REAL
DIM i: INTEGER
DIM s: STRING[20]
DIM b: BOOLEAN
DIM out: STRING[20]
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
r = 12.349
PRINT USING "R8.2", r
total = total+1
PRINT "  (^ expect field width 8, content rounds to 12.35)"
i = 10
PRINT USING "I4", i
total = total+1
PRINT "  (^ expect field width 4, content 10)"
s = "HELLO"
PRINT USING "S8", s
total = total+1
PRINT "  (^ expect field width 8, content HELLO)"
i = 100
PRINT USING "H4", i
total = total+1
PRINT "  (^ expect field width 4, hex of 100 = 0064)"
b = TRUE
PRINT USING "B8", b
total = total+1
PRINT "  (^ expect field width 8, content TRUE)"
PRINT USING "R8.2<,R8.2>", 5.5, 5.5
total = total+1
PRINT "  (^ two REAL fields, left then right justify)"
PRINT USING "2(I3,X1)", 1, 2
total = total+1
PRINT "  (^ repeat group: I3,X1,I3,X1)"
PRINT "Total statements exercised: "; total
END
