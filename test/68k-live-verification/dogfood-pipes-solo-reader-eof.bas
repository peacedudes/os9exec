PROCEDURE pipesoloR
! Dogfood task 2026-07-18: pipe IPC, testing common/ipc.md's EOF rule in
! isolation -- a single process, no second process involved at all.
!
! Claim under test: "EOF is empty AND no other writers" -- EOF is
! recognized once the pipe is empty AND reader count equals total user
! count. Here this process is the ONLY path holder ever: it CREATEs a
! fresh NAMED pipe for READ only and immediately tries to READ, with no
! writer ever going to open it in this run. Reader count (1) == total
! user count (1) and the pipe is empty from the start, so the rule
! predicts immediate EOF, not an indefinite block waiting for a writer
! that will never come.
!
! Gotcha found getting here: BASIC09's OPEN (mapped to a "must already
! exist" I$Open) does NOT auto-create a not-yet-existing named pipe --
! an earlier version of this file used OPEN and crashed uncaught with
! `001E Error #000:216 (E_PNNF) Path Name Not Found` the moment the shell
! forked it. CREATE is what actually instantiates the pipe (matches
! ipc.md's own framing: "Default buffer: 90 bytes, overridable via
! S_ISIZE option to _os_create()" -- creation, not open, is the mechanism
! that establishes a pipe). Not documented explicitly either way in
! ipc.md for the OPEN-vs-CREATE distinction on a not-yet-existing named
! pipe specifically.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", `pipesoloR` at the shell, fresh module, confirmed via `mdir`
! not to be resolving a stale resident copy from the earlier OPEN attempt):
!   soloR: opened /pipe/soloB for READ, no writer will ever open it
!   soloR: READ failed as expected, ERR=211
!   soloR: done
!
! Confirms the EOF rule cleanly: no writer ever touched this pipe, and the
! READ returned E$EOF (211) immediately rather than blocking forever.
DIM rp: BYTE
DIM line: STRING[80]
ON ERROR GOTO 800
CREATE #rp, "/pipe/soloB": READ
PRINT #2, "soloR: opened /pipe/soloB for READ, no writer will ever open it"
ON ERROR GOTO 900
READ #rp, line
PRINT #2, "soloR: unexpectedly read data: "; line
GOTO 999
800 PRINT #2, "soloR: CREATE failed, ERR="; ERR
GOTO 999
900 PRINT #2, "soloR: READ failed as expected, ERR="; ERR
999 PRINT #2, "soloR: done"
END
