! Resolves an OCR ambiguity in the manual: the PRINT USING justification
! table's third symbol rendered as "°" (degree sign) in the OCR
! text, illegible/uncertain as transcribed. Live-tested candidates on
! 68k: '%' is a runtime syntax error (Error #000:063); '^' compiles and
! CENTERS correctly -- confirmed via exact byte capture (file round-trip
! + LEN()), not eyeballing: "HI" in an 8-wide field gives "HI      "
! (left), "      HI" (right), "   HI   " (caret -- genuinely 3 spaces
! each side, centered). '^' is almost certainly what the manual's
! garbled degree-sign character actually was.
!
! LIVE RESULT (68k): left/right/caret all captured exactly as above.
PROCEDURE t_pujustify2
DIM path: BYTE
DIM out: STRING[20]
CREATE #path, "pujtest": WRITE
PRINT #path USING "S8<", "HI"
PRINT #path USING "S8>", "HI"
PRINT #path USING "S8^", "HI"
CLOSE #path
OPEN #path, "pujtest": READ
READ #path, out
PRINT "left  =["; out; "] LEN="; LEN(out)
READ #path, out
PRINT "right =["; out; "] LEN="; LEN(out)
READ #path, out
PRINT "caret =["; out; "] LEN="; LEN(out)
CLOSE #path
DELETE "pujtest"
END
