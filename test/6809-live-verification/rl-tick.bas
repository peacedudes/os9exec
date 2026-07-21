PROCEDURE rltick
(* Date-stamps a line roughly once a second for 60 seconds into a text
(* file open for update, so a concurrent `list` can chase it. Deletes any
(* leftover from a previous run itself, so a rerun is just: rltick.
DIM path: BYTE
DIM i, j: INTEGER
ON ERROR GOTO 10
DELETE "rltick.txt"
10 ON ERROR
CREATE #path, "rltick.txt": UPDATE
PRINT #2, "tick: start v2"
FOR i = 1 TO 60
  PRINT #path, "line "; i; " at "; DATE$
  FOR j = 1 TO 10000
  NEXT j
NEXT i
PRINT #2, "tick: closing"
CLOSE #path
END
