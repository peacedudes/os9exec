! Does an ON ERROR GOTO trap auto-clear after firing once? Some
! references claim "a trap clears itself after firing once" — this test
! settles it cleanly (finite, no runaway loop): trigger the SAME error
! (subscript out of range) TWICE in a row, with no re-arming `ON ERROR`
! call between the two triggers.
!
! CONFIRMED on 68k: the trap fires BOTH times (hits=1, then hits=2) with
! no re-arming — directly contradicting the "fires once" claim. The trap
! stays armed until explicitly disabled with a bare `ON ERROR`.
!
! LIVE RESULT (68k): "trap fired, hits=1" / "trap fired, hits=2" /
! "final hits=2"
PROCEDURE t_onerror4
DIM hits: INTEGER
DIM arr(5): INTEGER
hits = 0
ON ERROR GOTO 300
arr(99) = 1
PRINT "unreachable1"
300 hits = hits+1
PRINT "trap fired, hits="; hits
IF hits=1 THEN
 arr(99) = 2
 PRINT "unreachable2"
ENDIF
PRINT "final hits="; hits
END
