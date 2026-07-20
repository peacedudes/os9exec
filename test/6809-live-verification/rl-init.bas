PROCEDURE rlinit
(* Creates the shared one-record file the rl-hold/rl-wait pair contend on.
(* Record is 10 bytes: a counter plus filler, matching the earlier
(* lost-update fixtures so results stay comparable.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
CREATE #path, "rl.dat": UPDATE
FOR i = 1 TO 5
  rec(i) = 0
NEXT i
PUT #path, rec
CLOSE #path
PRINT #2, "rlinit: rl.dat created, count=0"
END
