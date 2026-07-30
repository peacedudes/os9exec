e cwareatest
 DIM p:INTEGER
 OPEN #p,"/w5":WRITE
 RUN GFX2(p,"DWSET",6,0,0,40,24,0,1,1)
 RUN GFX2(p,"SELECT")
 RUN GFX2(p,"CWAREA",0,0,20,12)
 RUN GFX2(p,"COLOR",3)
 RUN GFX2(p,"BOX",0,0,639,191)
 LOOP
 ENDLOOP
q
run cwareatest
bye
