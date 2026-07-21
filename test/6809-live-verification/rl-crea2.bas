PROCEDURE rlcreau
(* Update-mode CREATOR -- the control for rlcreaw. An update path holds
(* the eof lock legitimately, so a reader must follow this one right to
(* the close, exactly as it follows rlslowu. If gating Creat on the open
(* mode ever stops THIS follow, the gate broke the design.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i, j: INTEGER
DIM d: STRING[40]
CREATE #path, "rl.dat": UPDATE
PRINT #2, "creau: start v1"
FOR i = 1 TO 5
  d = DATE$
  rec(1) = i
  rec(2) = VAL(MID$(d,13,2))*60+VAL(MID$(d,16,2))
  PUT #path, rec
  PRINT #2, "creau: wrote record "; i; " at "; rec(2)
  FOR j = 1 TO 10000
  NEXT j
NEXT i
PRINT #2, "creau: closing"
CLOSE #path
END
