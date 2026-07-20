PROCEDURE rlhold2
(* Wide-window version of rlhold, for use at full emulation speed.
(*
(* rlhold's delay loops are short enough that the whole program can finish
(* before a companion process has even loaded, which makes the measurement
(* silently vacuous -- both sides report success and nothing was ever
(* contended. This version holds the record long enough that rlwait is
(* certain to arrive while the lock is genuinely held.
(*
(* Read the result the same way as rlhold: where rlwait's "GET returned"
(* marker lands says which operation released the lock.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i, j: INTEGER
OPEN #path, "rl.dat": UPDATE
SEEK #path, 0
GET #path, rec
PRINT #2, "hold2: GET done, count="; rec(1); " -- lock held"
FOR i = 1 TO 300
  FOR j = 1 TO 2000
  NEXT j
NEXT i
rec(1) = rec(1) + 1
SEEK #path, 0
PUT #path, rec
PRINT #2, "hold2: PUT done, count="; rec(1); " -- lock should now be free"
FOR i = 1 TO 300
  FOR j = 1 TO 2000
  NEXT j
NEXT i
PRINT #2, "hold2: closing"
CLOSE #path
END
