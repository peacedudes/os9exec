PROCEDURE pipeprod
! Dogfood task 2026-07-18: pipe IPC, testing common/ipc.md's "Pipes" section.
! Paired with dogfood-pipes-consumer.bas. Run together as a real OS-9 shell
! pipeline: `pipeprod ! pipecons` (both PACKed BASIC09 procedures, run via
! RunB through the shell's bare-name auto-detection -- see pack-and-runb.md).
!
! Claim under test: ipc.md says the default (unnamed) pipe buffer is 90
! bytes. This producer issues THREE separate WRITE calls, each carrying a
! single 100-byte string (101 bytes on the wire once BASIC09 appends the
! record-terminating CR) -- i.e. every single WRITE already exceeds the
! documented 90-byte default on its own, well before the second or third
! call. ERR is printed after each WRITE to catch any error the call might
! raise (short write, E_WRITE, etc).
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", `pipeprod ! pipecons &` at the shell -- see
! dogfood-report-pipes-2026-07-18.md for the full session):
!   producer: chunk length=100
!   producer: first WRITE returned, ERR=0
!   producer: second WRITE returned, ERR=0
!   producer: third WRITE returned, ERR=0
!   producer: sentinel written, exiting now (closes pipe)
! -- all three WRITEs (303 bytes of payload + 3 CRs = 306 bytes, before
! even counting the sentinel) returned ERR=0. No short write, no E_WRITE,
! no truncation. See dogfood-pipes-consumer.bas's header for what the
! reader side saw -- all 309 payload bytes arrived intact.
DIM chunk: STRING[120]
DIM i: INTEGER
chunk = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
PRINT #2, "producer: chunk length="; LEN(chunk)
WRITE #1, chunk
PRINT #2, "producer: first WRITE returned, ERR="; ERR
WRITE #1, chunk
PRINT #2, "producer: second WRITE returned, ERR="; ERR
WRITE #1, chunk
PRINT #2, "producer: third WRITE returned, ERR="; ERR
WRITE #1, "ENDOFDATA"
PRINT #2, "producer: sentinel written, exiting now (closes pipe)"
END
