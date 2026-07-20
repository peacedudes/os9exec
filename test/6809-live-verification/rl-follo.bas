PROCEDURE rlfollo
(* Read-only chaser. Prints, for every record it gets, the time the record
(* was written and the time it arrived here. The gap between the two is the
(* measurement: following properly means arriving a moment behind the
(* writer, so the gap stays small.
(*
(* A large gap means this was parked while the producer went on writing --
(* it did not follow, it waited and then caught up in a rush at the end.
(*
(* No error trapping on purpose: dying on end of file, and when, is also a
(* result. Dying while the producer still runs means it stopped at the end
(* that existed instead of following it.
(*
(* Deliberately built from FOR and PRINT only. WHILE/ENDWHILE and the
(* multi-line IF/ELSE/ENDIF both fail to compile in this BASIC09 with
(* "Unmatched Control Structure", which is what silently broke rl-grow.
DIM path: BYTE
DIM rec(5): INTEGER
DIM n, now: INTEGER
DIM d: STRING[40]
OPEN #path, "rl.dat": READ
PRINT #2, "follo: opened"
FOR n = 1 TO 99
  GET #path, rec
  d = DATE$
  now = VAL(MID$(d,13,2)) * 60 + VAL(MID$(d,16,2))
  PRINT #2, "follo: record "; rec(1); " written "; rec(2); " read "; now
NEXT n
CLOSE #path
END
