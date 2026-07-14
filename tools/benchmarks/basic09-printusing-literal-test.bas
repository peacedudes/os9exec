! Confirms a literal string control-spec combined with a value directive
! in one PRINT USING format string works as documented -- used to fix
! this skill's flagship "grades" worked example, which previously used
! the fabricated "###.##" syntax.
!
! LIVE RESULT (68k): "Average:  82.57" (82.567 correctly rounds to 82.57)
PROCEDURE t_printusing5
DIM avg: REAL
avg = 82.567
PRINT USING "'Average: ',R6.2", avg
END
