! ON ERROR GOTO with a real, explicit numbered line and a genuine
! software-level error (subscript out of range) — confirmed WORKING on
! 68k: control jumps to the trap line, ERR holds the correct error code,
! and "PRINT unreachable" is correctly skipped. `ERR` correctly reads 55
! (Subscript out of Range per the manual's error-code appendix).
!
! NOTE: an earlier attempt used integer division by zero to trigger the
! trap and found it does NOT raise any error at all (silent fall-through)
! — see basic09-onerror-refire-test.bas and basic09-language.md's Error
! Handling section for that finding plus the much bigger one (REAL
! divide-by-zero crashes the whole process, uncatchable by ON ERROR).
!
! LIVE RESULT (68k): "ERR=55" then "trapcount=1"
PROCEDURE t_onerror3
DIM trapcount: INTEGER
DIM arr(5): INTEGER
trapcount = 0
ON ERROR GOTO 300
arr(99) = 1
PRINT "unreachable"
300 trapcount = trapcount+1
PRINT "ERR="; ERR
ON ERROR
PRINT "trapcount="; trapcount
END
