! IF Statement Type 2 -- ENDIF must be on its own line. Cramming it onto
! the same line as the THEN-clause statement (`IF cond THEN GOTO 10
! ENDIF` all one line) is ALSO invalid -- a different error
! (Error #000:011), tested and confirmed separately. This is the real,
! working form: ENDIF on its own following line.
!
! LIVE RESULT (68k): "type2 done i=5"
PROCEDURE t_iftype2
DIM i: INTEGER
i = 0
10 i = i+1
IF i<5 THEN
 GOTO 10
ENDIF
PRINT "type2 done i="; i
END
