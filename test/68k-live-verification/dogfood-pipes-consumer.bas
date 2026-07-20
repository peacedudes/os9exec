PROCEDURE pipecons
! Dogfood task 2026-07-18: pipe IPC, testing common/ipc.md's "Pipes" section.
! Paired with dogfood-pipes-producer.bas. Run together as a real OS-9 shell
! pipeline: `pipeprod ! pipecons`.
!
! Claims under test:
!  1. The producer writes far more than the documented 90-byte default
!     buffer in three separate WRITE calls, before this consumer has read
!     anything (a busy-wait FOR/NEXT loop delays the first READ). Does the
!     data survive intact, and does the shell's `!` unnamed-pipe mechanism
!     actually connect the two processes' stdout/stdin as documented?
!  2. EOF: ipc.md claims EOF is recognized only once the pipe is empty AND
!     reader count equals total user count -- i.e. a clean EOF once the
!     writer closes (exits) and all buffered data has been drained, not
!     before. `READ` past that point should raise E$EOF (error 211).
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", `pipeprod ! pipecons &` at the shell):
!   consumer: starting delay before first read
!   consumer: delay done, beginning reads
!   consumer: read #1 len=100 data=0123456789...(100 digits)
!   consumer: read #2 len=100 data=0123456789...(100 digits)
!   consumer: read #3 len=100 data=0123456789...(100 digits)
!   consumer: read #4 len=9 data=ENDOFDATA
!   consumer: read failed with ERR=211 after 4 successful reads, 309 total data chars
!
! Confirms both claims: all 309 payload bytes (100+100+100+9) arrived
! intact and in order via the shell's `!` pipe despite the producer having
! written 306+ bytes -- more than 3x the documented 90-byte default --
! before this reader ever touched the pipe. The 5th READ (past the
! sentinel, pipe now empty and the writer already exited) raised ERR=211
! (E$EOF) exactly as ipc.md's EOF rule describes -- not a hang, not a
! different error code, not silent zero-length success.
DIM line: STRING[200]
DIM n, totalchars, k: INTEGER
PRINT #2, "consumer: starting delay before first read"
FOR k = 1 TO 400000
NEXT k
PRINT #2, "consumer: delay done, beginning reads"
n = 0
totalchars = 0
ON ERROR GOTO 900
100 READ #0, line
n = n + 1
totalchars = totalchars + LEN(line)
PRINT #2, "consumer: read #"; n; " len="; LEN(line); " data="; line
GOTO 100
900 PRINT #2, "consumer: read failed with ERR="; ERR; " after "; n; " successful reads, "; totalchars; " total data chars"
END
