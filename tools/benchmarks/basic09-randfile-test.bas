! Random-access file I/O (PUT/GET/SEEK) with a TYPE record -- previously
! only documented from the Tandy manual's Disk Files chapter, never
! actually run. Confirmed working on 68k: pre-allocate 5 empty records,
! SEEK to a computed offset (2*SIZE(record)), PUT a record there, SEEK
! back and GET it, verify round-trip; also verify an untouched
! pre-allocated record elsewhere in the file is still empty (confirms
! SEEK+PUT only touches the targeted record, not neighbors).
!
! LIVE RESULT (68k): 2/2 PASS
PROCEDURE t_randfile
TYPE rec = name: STRING[10]; qty: INTEGER
DIM work: rec
DIM path: BYTE
DIM pass,fail,total: INTEGER
pass = 0
fail = 0
total = 0
CREATE #path, "randtest"
work.name = ""
work.qty = 0
FOR n = 1 TO 5
 PUT #path, work
NEXT n
work.name = "widget"
work.qty = 42
SEEK #path, 2 * SIZE(work)
PUT #path, work
work.name = ""
work.qty = 0
SEEK #path, 2 * SIZE(work)
GET #path, work
total = total+1
IF work.name="widget" AND work.qty=42 THEN
 pass = pass+1
 PRINT "PASS: SEEK+PUT+GET round-trips a TYPE record at a computed offset"
ELSE
 fail = fail+1
 PRINT "FAIL: got name=["; work.name; "] qty="; work.qty
ENDIF
work.name = ""
work.qty = 0
SEEK #path, 0
GET #path, work
total = total+1
IF work.name="" AND work.qty=0 THEN
 pass = pass+1
 PRINT "PASS: record 1 is still the pre-allocated empty record"
ELSE
 fail = fail+1
 PRINT "FAIL: record 1 got name=["; work.name; "] qty="; work.qty
ENDIF
CLOSE #path
DELETE "randtest"
PRINT "Total: "; total; " Pass: "; pass; " Fail: "; fail
END
