! REAL COMPILER BUG in real 68k BASIC09: this is the minimal possible
! repro. A single-line `IF cond THEN GOTO n` (no ENDIF) always fails to
! compile with Error #000:069 (Unmatched Control Structure) -- even
! standing completely alone, nothing else in the procedure. Bisected via
! basic09-oneline-if-*-variants (not all committed -- see the write-up
! in basic09-language.md's Control Structures section and gotchas.md):
! NOT about jump direction (forward-targeting fails identically to
! backward), NOT GOTO-specific (IF cond THEN GOSUB n fails the same
! way), NOT about combining with other constructs (fails alone). The
! BLOCK form (IF cond THEN / GOTO n / ENDIF) always works -- use it,
! always, for any conditional GOTO/GOSUB. This also explains an earlier,
! messier "GOTO+GOSUB+ON ERROR GOTO combined" finding that turned out to
! be the exact same bug wearing a more complicated disguise.
!
! LIVE RESULT (68k): Error #000:069 at LOAD time, every time.
PROCEDURE t_biF
DIM i: INTEGER
i = 0
10 i = i+1
IF i<5 THEN GOTO 10
PRINT "F done i="; i
END
