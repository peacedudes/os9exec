PROCEDURE rlchk2
(* Like rlcheck but also prints rec(5), the racers-done marker, so a harness
(* can trust a run (done=2) without scraping the racers' own output lines.
DIM path: BYTE
DIM rec(5): INTEGER
OPEN #path, "rl.dat": READ
SEEK #path, 0
GET #path, rec
CLOSE #path
PRINT #2, "chk2: counter="; rec(1); " done="; rec(5); " lost="; 400 - rec(1)
END
