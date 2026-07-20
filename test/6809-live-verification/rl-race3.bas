PROCEDURE rlrace3
(* rlrace with a self-reported completion count, so a run can be trusted.
(*
(* Reading the counter alone cannot tell a lost update from a racer that had
(* not finished yet, and at these speeds the shell returns long before the
(* backgrounded racer is done -- which makes every short read look like data
(* loss. Each racer now bumps rec(5) as its last act, under the same lock.
(*
(* A run counts only when rec(5) = 2. Then rec(1) = 400 exactly, or the
(* difference is the number of genuinely lost updates.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
OPEN #path, "rl.dat": UPDATE
FOR i = 1 TO 200
  SEEK #path, 0
  GET #path, rec
  rec(1) = rec(1) + 1
  SEEK #path, 0
  PUT #path, rec
NEXT i
SEEK #path, 0
GET #path, rec
rec(5) = rec(5) + 1
SEEK #path, 0
PUT #path, rec
CLOSE #path
PRINT #2, "race3: finished 200, racers done="; rec(5)
END
