! Numeric type widening/division rules — confirmed on 68k. Key finding:
! division is computed from the OPERAND types, not the destination
! variable's type. `i / 3` (both INTEGER) truncates to INTEGER division
! BEFORE the result is widened into a REAL destination — assigning to a
! REAL variable does not retroactively force real division. A REAL
! literal operand (`i / 3.0`) does force real division. BYTE+INTEGER
! widens correctly to REAL, and an integer literal with no decimal point
! assigned to a REAL var becomes X.0 as expected.
!
! LIVE RESULT (68k): all 5 checks PASS
PROCEDURE t_numwiden2
DIM b: BYTE
DIM i: INTEGER
DIM r: REAL
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
b = 200
i = 100000
r = b + i
total = total+1
IF r = 100200 THEN
 pass = pass+1
 PRINT "PASS: BYTE+INTEGER widens correctly, r="; r
ELSE
 fail = fail+1
 PRINT "FAIL: BYTE+INTEGER got r="; r
ENDIF
i = 100000
r = i / 3
total = total+1
IF r = 33333 THEN
 pass = pass+1
 PRINT "PASS: INTEGER/INTEGER truncates BEFORE widening to REAL dest, r="; r
ELSE
 fail = fail+1
 PRINT "FAIL: INTEGER/INTEGER->REAL got r="; r
ENDIF
r = i / 3.0
total = total+1
IF r > 33333 AND r < 33334 THEN
 pass = pass+1
 PRINT "PASS: a REAL literal divisor forces REAL division, r="; r
ELSE
 fail = fail+1
 PRINT "FAIL: INTEGER/REAL-literal got r="; r
ENDIF
i = 7 / 2
total = total+1
IF i = 3 THEN
 pass = pass+1
 PRINT "PASS: INTEGER/INTEGER->INTEGER truncates, i="; i
ELSE
 fail = fail+1
 PRINT "FAIL: INTEGER/INTEGER->INTEGER got i="; i
ENDIF
r = 5
total = total+1
IF r = 5.0 THEN
 pass = pass+1
 PRINT "PASS: literal 5 (no decimal point) assigned to REAL var becomes 5.0"
ELSE
 fail = fail+1
 PRINT "FAIL: r="; r
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
