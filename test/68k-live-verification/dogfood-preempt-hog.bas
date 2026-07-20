! A loop that computes for about five seconds and makes no system calls at
! all -- so nothing in OS9exec takes the CPU away from it, because nothing
! ever did: a process here runs until it traps or faults. Paired with
! dogfood-preempt-talker.bas, which just prints and exits.
!
!   run as:  hog & talk
!
! Without the system tick the talker cannot run until the hog is completely
! finished -- "hog: finished" appears BEFORE any "talk: alive" line. With -q
! the talker runs immediately and the hog finishes five seconds later. The
! order of the two is the whole result; the timestamps only confirm it.
!
! Measured 2026-07-19:
!   default:  hog: finished 17:02:27  then talk 1..6 at 17:02:27
!   -q:       talk 1..6 at 17:02:27   then hog: finished 17:02:32
PROCEDURE hog
DIM i, j: INTEGER
FOR i = 1 TO 40
FOR j = 1 TO 250000
NEXT j
NEXT i
PRINT #2, "hog: finished at ", RIGHT$(DATE$,8)
END
