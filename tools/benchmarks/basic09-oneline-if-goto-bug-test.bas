! CORRECTED (was wrongly filed as "a real compiler bug"): this is
! INVALID SYNTAX, not a bug. The manual defines two distinct IF forms --
! Type 1 `IF cond THEN linenum` (bare line number, NO "GOTO" keyword,
! no ENDIF) and Type 2 `IF cond THEN <statements> ENDIF` (ENDIF is
! mandatory, unlike the bracketed-optional ELSE). `IF cond THEN GOTO 10`
! below has the GOTO keyword (illegal in Type 1) AND omits ENDIF
! (required in Type 2) -- it's neither form, a hybrid that was never
! valid syntax. The compiler is CORRECTLY rejecting it with
! Error #000:069. See basic09-iftype1-test.bas and
! basic09-iftype2-test.bas for the two real, working forms, and
! basic09-language.md's Control Structures section for the full
! grammar. Kept as a negative test: confirms this specific invalid
! syntax is (still) correctly rejected, not accidentally accepted.
!
! LIVE RESULT (68k): Error #000:069 at LOAD time, every time -- CORRECT
! rejection of invalid syntax, not a bug.
PROCEDURE t_biF
DIM i: INTEGER
i = 0
10 i = i+1
IF i<5 THEN GOTO 10
PRINT "F done i="; i
END
