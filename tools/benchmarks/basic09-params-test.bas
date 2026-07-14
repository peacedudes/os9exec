! Parameter-passing modes -- previously only documented from the BNF
! cards, never live-tested with an actual multi-procedure call. Confirmed
! on 68k: a plain variable name in a RUN call passes BY REFERENCE (the
! callee mutating its PARAM mutates the caller's variable); the
! by-value-forcing trick (`x+0` for numerics, `s$+""` for strings) works
! exactly as documented, insulating the caller's variable from the
! callee's mutation. Tested for both INTEGER and STRING parameters.
!
! LIVE RESULT (68k): 4/4 PASS
PROCEDURE t_params
DIM x: INTEGER
DIM s: STRING[10]
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
x = 5
RUN bumpref(x)
total = total+1
IF x=6 THEN
 pass = pass+1
 PRINT "PASS: plain var param passes BY REFERENCE, caller x mutated to "; x
ELSE
 fail = fail+1
 PRINT "FAIL: by-ref x="; x
ENDIF
x = 5
RUN bumpval(x+0)
total = total+1
IF x=5 THEN
 pass = pass+1
 PRINT "PASS: x+0 trick forces BY VALUE, caller x unchanged at "; x
ELSE
 fail = fail+1
 PRINT "FAIL: by-val x="; x
ENDIF
s = "hi"
RUN appendref(s)
total = total+1
IF s="hiX" THEN
 pass = pass+1
 PRINT "PASS: string var param passes BY REFERENCE, caller s mutated to ["; s; "]"
ELSE
 fail = fail+1
 PRINT "FAIL: by-ref s=["; s; "]"
ENDIF
s = "hi"
RUN appendval(s+"")
total = total+1
IF s="hi" THEN
 pass = pass+1
 PRINT "PASS: s+"""" trick forces BY VALUE, caller s unchanged at ["; s; "]"
ELSE
 fail = fail+1
 PRINT "FAIL: by-val s=["; s; "]"
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END

PROCEDURE bumpref
PARAM n: INTEGER
n = n + 1
END

PROCEDURE bumpval
PARAM n: INTEGER
n = n + 1
END

PROCEDURE appendref
PARAM t: STRING[10]
t = t + "X"
END

PROCEDURE appendval
PARAM t: STRING[10]
t = t + "X"
END
