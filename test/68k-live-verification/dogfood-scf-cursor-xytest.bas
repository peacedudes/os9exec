PROCEDURE xytest
! Captured 2026-07-30 from the live 68k system disk (/dd/USR/DOG/xytest),
! which is a working master and not under version control -- this was the
! only copy. Run under BASIC09 on os9exec; a host-side capture decodes the
! 1200 positioned characters to prove SCF dropped, duplicated or reordered
! nothing. The trailing XYTEST-END marks a complete run.
! Dense SCF cursor test: one positioned character per cell, so a host-side
! decode can prove nothing was dropped, duplicated or reordered.
DIM r,c,v:INTEGER
RUN clrscr
FOR r=1 TO 20
  FOR c=1 TO 60
    RUN xy(r,c)
    v=MOD(r*60+c,90)
    PRINT CHR$($21+v);
  NEXT c
NEXT r
RUN xy(22,1)
PRINT "XYTEST-END";
PROCEDURE xy
PARAM x,y:INTEGER
IF x<1 THEN x=1 \ ENDIF
IF x>24 THEN x=24 \ ENDIF
IF y<1 THEN y=1 \ ENDIF
IF y>80 THEN y=80 \ ENDIF
PRINT CHR$($1B);"[";
RUN num(x)
PRINT ";";
RUN num(y)
PRINT "H";
PROCEDURE num
! PRINT of an INTEGER emits a leading space for the sign, which would
! corrupt the escape sequence. Emit bare digits.
PARAM n:INTEGER
DIM t:INTEGER
t=INT(n/10)
IF t>0 THEN PRINT CHR$($30+t); \ ENDIF
PRINT CHR$($30+(n-10*t));
PROCEDURE clrscr
PRINT CHR$($1B);"[2J";CHR$($1B);"[H";
