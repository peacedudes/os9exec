PROCEDURE tally
DIM rp: BYTE
DIM line$: STRING[128]
DIM pcount, fcount, scount, ecount: INTEGER
pcount = 0
fcount = 0
scount = 0
ecount = 0
ON ERROR GOTO 800
OPEN #rp, "/x0/RESULTS/report": READ
ON ERROR GOTO 900
LOOP
  READ #rp, line$
  IF SUBSTR("PASS",line$) <> 0 THEN
    pcount = pcount + 1
  ENDIF
  IF SUBSTR("FAIL",line$) <> 0 THEN
    fcount = fcount + 1
  ENDIF
  IF SUBSTR("SKIP",line$) <> 0 THEN
    scount = scount + 1
  ENDIF
  IF SUBSTR("ERROR",line$) <> 0 THEN
    ecount = ecount + 1
  ENDIF
ENDLOOP
900 ON ERROR GOTO 950
CLOSE #rp
950 ON ERROR
PRINT "CONF6809 totals: PASS=";pcount;" FAIL=";fcount;" SKIP=";scount;" ERROR=";ecount
END
800 ON ERROR
PRINT "CONF6809 totals: PASS=0 FAIL=0 SKIP=0 ERROR=0  (no report yet)"
END
