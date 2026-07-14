! DATE$ byte-level dump via ASC(MID$(...)) -- bypasses terminal display
! entirely (a plain PRINT DATE$ looked like it started with "<" instead
! of a digit, which could have been a terminal-rendering artifact; this
! test settles it definitively via ASC()).
!
! REAL BUG FOUND: DATE$'s leading year-tens-digit is WRONG. On a day
! when the real date was 2026-07-14, DATE$ returned "<6/07/14 HH:MM:SS"
! -- ASC() of the first character is 60 ('<'), not 50 ('2'). Expected
! "26/07/14 ...". The +10 offset (60 vs 50) is consistent with a
! BCD/decade-offset bug: computing the tens digit from an unreduced
! "126" (a since-1900 year offset, 2026-1900=126) instead of "26"
! (126/10=12, 12+ASCII'0'(48)=60='<' -- exactly matches). Whether this
! bug lives in BASIC09's own DATE$ runtime code (would reproduce on real
! 6809 hardware) or in os9exec's kernel-level date/time emulation
! (emulator-specific, not a BASIC09 language bug at all) is NOT yet
! determined -- worth checking both. Documented in basic09-language.md.
!
! LIVE RESULT (68k): 17 ASC() values printed; decode confirms the '<'
! leading byte and the BCD-offset hypothesis.
PROCEDURE t_date3
DIM d: STRING[32]
DIM i,n: INTEGER
d = DATE$
n = LEN(d)
FOR i = 1 TO n
 PRINT i; ":"; ASC(MID$(d,i,1)); " "
NEXT i
END
