! IF Statement Type 1 (manual's own term): `IF cond THEN linenum` -- a
! BARE line number after THEN, no "GOTO" keyword at all, no ENDIF. This
! is the real classic-BASIC-style conditional jump. Confirms an earlier
! session's finding of "IF cond THEN GOTO n always fails, real compiler
! bug" was wrong -- that form was an invalid hybrid (has the illegal
! GOTO keyword AND omits the required ENDIF). This bare-linenum Type 1
! form is what real BASIC09 actually provides for a single-line
! conditional jump.
!
! LIVE RESULT (68k): "type1 done i=5"
PROCEDURE t_iftype1
DIM i: INTEGER
i = 0
10 i = i+1
IF i<5 THEN 10
PRINT "type1 done i="; i
END
