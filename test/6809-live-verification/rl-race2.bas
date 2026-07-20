PROCEDURE rlrace2
(* Diagnostic twin of rlrace: identical read-modify-write contention, but
(* the path is closed and reopened around every cycle.
(*
(* rlrace keeps one path open and returns to the same sector 200 times, so
(* a path-local cached sector would be reused across a sibling's write.
(* Closing drops that cache. If this scores 400 where rlrace does not, the
(* residual losses are stale cached data, not a failure of the lock.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
FOR i = 1 TO 200
  OPEN #path, "rl.dat": UPDATE
  SEEK #path, 0
  GET #path, rec
  rec(1) = rec(1) + 1
  SEEK #path, 0
  PUT #path, rec
  CLOSE #path
NEXT i
PRINT #2, "race2: this racer finished its 200"
END
