PROCEDURE rltick
      (* Captured 2026-07-30 from the live 68k system disk
      (* (/dd/USR/DOG/rltick), a working master not under version control --
      (* this was the only copy. The 68k RBF EOF-lock ticker: distinct code
      (* from test/6809-live-verification/rl-tick.bas, which is the 6809
      (* variant of the same idea, not a copy of this.
      (* Date-stamps a line each second for 60 seconds into a text file open
      (* for update, so a concurrent `list` chases it, parked on the eof lock.
      (* Paces with a real F$Sleep (code 10, ticks in X, 60/s), and deletes
      (* any leftover from a previous run itself: a rerun is just rltick.
      TYPE registers=cc,a,b,dp:BYTE; x,y,u:INTEGER
      DIM regs:registers
      DIM path:BYTE
      DIM i:INTEGER
      ON ERROR GOTO 10
      DELETE "rltick.txt"
 10   ON ERROR
      CREATE #path,"rltick.txt":UPDATE
      PRINT #2,"tick: start v3"
      FOR i=1 TO 60
        PRINT #path,"line "; i; " at "; DATE$
        regs.x=60
        RUN syscall(10,regs)
      NEXT i
      PRINT #2,"tick: closing"
      CLOSE #path
      END
