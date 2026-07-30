e fontfg
 DIM p:INTEGER
 OPEN #p,"/w6":WRITE
 PRINT "A"
 RUN GFX2(p,"DWSET",6,0,0,40,24,0,1,1)
 PRINT "B"
 RUN GFX2(p,"FONT",200,1)
 PRINT "C"
 CLOSE #p
 PRINT "D"
q
run fontfg
bye
