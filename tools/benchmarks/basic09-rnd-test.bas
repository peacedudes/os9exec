! RND() reseeding determinism -- previously only RND(0)'s range was
! tested. Confirmed on 68k: RND(n>0) returns a REAL in [0,n) -- NOT
! necessarily an integer (RND(5) returned 1.75959429, a common
! assumption from other BASICs that doesn't hold here). RND(n<0) uses
! ABS(n) as a new seed (per the manual). Reseeding is fully
! deterministic: calling RND(-42) twice, each followed by the same 3
! RND(0) calls, produces the EXACT SAME 3-value sequence both times --
! useful for anyone needing reproducible test data or deterministic
! simulation runs.
!
! LIVE RESULT (68k): 3/3 PASS
PROCEDURE t_rnd
DIM x,y,z: REAL
DIM a1,a2,a3,b1,b2,b3: REAL
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
x = RND(5)
total = total+1
IF x>=0 AND x<5 THEN
 pass = pass+1
 PRINT "PASS: RND(5) in range [0,5), got "; x
ELSE
 fail = fail+1
 PRINT "FAIL: RND(5) out of range, got "; x
ENDIF
y = RND(-42)
total = total+1
IF y>=0 THEN
 pass = pass+1
 PRINT "PASS: RND(-42) reseed call returns a value >=0, got "; y
ELSE
 fail = fail+1
 PRINT "FAIL: RND(-42) got "; y
ENDIF
y = RND(-42)
a1 = RND(0)
a2 = RND(0)
a3 = RND(0)
y = RND(-42)
b1 = RND(0)
b2 = RND(0)
b3 = RND(0)
total = total+1
IF a1=b1 AND a2=b2 AND a3=b3 THEN
 pass = pass+1
 PRINT "PASS: same negative seed reproduces the identical sequence"
ELSE
 fail = fail+1
 PRINT "FAIL: sequences differ. a=["; a1; ","; a2; ","; a3; "] b=["; b1; ","; b2; ","; b3; "]"
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
