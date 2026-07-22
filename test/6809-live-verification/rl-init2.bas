PROCEDURE rlinit2
(* Like rlinit, but writes TWO 10-byte records so rl.dat is 20 bytes.  The
(* racer works on record 0 (position 0), which is then NOT at end-of-file.  If
(* the lost update is specific to a record AT eof (the GET taking the EOF ghost
(* lock instead of a record lock), this bigger file should make the SAME racer
(* on STOCK RBF lose nothing -- a clean, uninstrumented test of that root cause.
DIM path: BYTE
DIM rec(5): INTEGER
DIM i: INTEGER
CREATE #path, "rl.dat": UPDATE
FOR i = 1 TO 5
  rec(i) = 0
NEXT i
PUT #path, rec
PUT #path, rec
CLOSE #path
PRINT #2, "rlinit2: rl.dat created 20 bytes, count=0"
END
