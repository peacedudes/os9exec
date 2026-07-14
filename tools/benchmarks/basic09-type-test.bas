! TYPE records test — confirmed WORKING on 68k with the single-line
! semicolon-separated grammar (`TYPE name = field: type; field: type`).
! An earlier finding claimed a fabricated multi-line TYPE/ENDTYPE block
! form was broken; that form never existed in either manual (zero grep
! hits for ENDTYPE). This test covers simple fields, nested TYPE-in-TYPE
! dot notation, and array-of-TYPE field access — all PASS.
!
! LIVE RESULT (68k):
!   PASS: simple TYPE field assign
!   PASS: nested TYPE dot notation
!   PASS: array-of-TYPE field access
!   Total: 3  Pass: 3  Fail: 0
PROCEDURE t_type
TYPE point = x,y: INTEGER
TYPE line = a,b: point; label: STRING[8]
DIM p1: point
DIM ln: line
DIM pts(3): point
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
p1.x = 10
p1.y = 20
total = total+1
IF p1.x=10 AND p1.y=20 THEN
 pass = pass+1
 PRINT "PASS: simple TYPE field assign"
ELSE
 fail = fail+1
 PRINT "FAIL: simple TYPE field assign"
ENDIF
ln.a.x = 1
ln.a.y = 2
ln.b.x = 3
ln.b.y = 4
ln.label = "seg1"
total = total+1
IF ln.a.x=1 AND ln.b.y=4 AND ln.label="seg1" THEN
 pass = pass+1
 PRINT "PASS: nested TYPE dot notation"
ELSE
 fail = fail+1
 PRINT "FAIL: nested TYPE dot notation"
ENDIF
pts(1).x = 100
pts(2).x = 200
total = total+1
IF pts(1).x=100 AND pts(2).x=200 THEN
 pass = pass+1
 PRINT "PASS: array-of-TYPE field access"
ELSE
 fail = fail+1
 PRINT "FAIL: array-of-TYPE field access"
ENDIF
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
