! Backs: os9-dev/references/basic09/basic09-language.md, INTEGER÷0 passage
! Tests: whether INTEGER÷0 silently falls through or raises an error on 6809
!
! Result observed on NitrOS-9 (6809) BASIC09:
!   *000E      i=i/0
!   Error #045 -- Divide by Zero
!   BREAK: PROCEDURE intdivtest
!   D:
! i.e. INTEGER÷0 on 6809 raises Error #045 and drops to Debug Mode -- NOT
! a silent fall-through. (The previously-documented "silently falls
! through" claim was correct for 68k, wrongly generalized to 6809.)

PROCEDURE intdivtest
DIM i: INTEGER
i = 5
i = i / 0
PRINT "After division, i =", i
END
