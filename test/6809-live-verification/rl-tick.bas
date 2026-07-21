PROCEDURE rltick
(* The owner's scenario, verbatim: date-stamps a line roughly once a
(* second for 60 seconds into a text file open for update, so a
(* concurrent `list` can chase it. The delay loop is the same 10000-count
(* body rlsloww uses, which paces at about a second on this machine.
DIM path: BYTE
DIM i, j: INTEGER
CREATE #path, "rltick.txt": UPDATE
PRINT #2, "tick: start v1"
FOR i = 1 TO 60
  PRINT #path, "line "; i; " at "; DATE$
  FOR j = 1 TO 10000
  NEXT j
NEXT i
PRINT #2, "tick: closing"
CLOSE #path
END
