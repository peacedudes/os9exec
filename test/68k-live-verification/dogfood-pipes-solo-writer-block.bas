PROCEDURE pipesoloW
! Dogfood task 2026-07-18: pipe IPC, testing common/ipc.md's named-vs-unnamed
! deadlock-detection distinction in isolation -- a single process, no
! second process involved at all.
!
! ipc.md's deadlock-detection claim is explicitly scoped to UNNAMED pipes:
! "For an unnamed pipe, if every process with access to it is
! simultaneously blocked trying to write ... OS-9 detects the resulting
! deadlock and returns E_WRITE ... rather than hanging forever." For NAMED
! pipes the same file says only: "Writing to a full named pipe blocks
! until space frees (unless the writer is interrupted by a signal)" -- no
! deadlock detection mentioned. This program CREATEs a fresh named pipe
! for WRITE only, with no reader EVER going to open it in this run (a
! degenerate, permanent deadlock case: the lone process with access is
! the writer itself, blocked, and nothing can ever free space), and writes
! 50 chunks of 100 bytes each in a loop, printing progress after each
! WRITE so the point of failure/blocking is visible.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude", `pipesoloW` at the shell -- run in the FOREGROUND, i.e. the
! shell itself blocks on it, matching the real interactive-use case):
!   soloW: created /pipe/soloC for WRITE, no reader will ever open it
!   soloW: chunk 1 written OK
!   soloW: chunk 2 written OK
!   ...
!   soloW: chunk 40 written OK
!   [stuck here -- no further output, no shell prompt, for 45+ real seconds]
!
! Two findings, both worth flagging:
!
! 1. The claim itself held up, but with a nuance not spelled out in
!    ipc.md: this run accepted 40 chunks (4000 payload bytes + 40 CRs =
!    4040 bytes) into a pipe with a documented 90-byte default buffer and
!    ZERO readers ever, all with ERR=0, before finally blocking on chunk
!    41's WRITE. That's ~45x the documented default before any blocking
!    was observed. Root cause not confirmed -- most likely BASIC09's own
!    WRITE statement batches several logical WRITE#s into fewer real
!    I$Write syscalls client-side (a runtime-level buffer sitting above
!    the kernel-level pipe buffer ipc.md describes), rather than the
!    kernel's default pipe buffer actually being ~4KB. Not independently
!    verified via syscall-level tracing (idbg -d 2) -- out of scope for
!    this pass. Whatever the mechanism, once truly full, it blocked
!    exactly as documented: no error, no truncation, no silent drop.
!
! 2. It never returned. No E_WRITE, no timeout, no deadlock detection --
!    consistent with ipc.md's named/unnamed distinction (deadlock
!    detection is claimed only for unnamed pipes). But recovering from it
!    live was a real problem the skill doesn't warn about: both Ctrl-C and
!    Ctrl-E (sent twice each, via the REPL's raw `key C-c`/`key C-e`, per
!    `using-os9exec-repl.md`'s "Stopping a runaway program" section, which
!    says Ctrl-E "kills the child ... regardless of what the process is
!    doing (compute loop, blocked write -- no console read needed)") had
!    NO visible effect after 45+ seconds -- the process stayed wedged at
!    chunk 40 and the shell prompt never returned. Recovery required a
!    full `./tools/os9repl.sh restart` (session state, not this program's
!    logic, was lost). This contradicts the skill's specific claim that
!    Ctrl-E works on a process blocked in a write. Not root-caused further
!    (could be a genuine os9exec/UAE-core bug in signal delivery to a
!    process parked deep in a kernel-level pipe wait, or could be correct
!    real-OS-9 behavior that the skill's Ctrl-E claim overstates for this
!    specific blocking case) -- flagged for follow-up, not fixed here
!    (out of scope for this dogfood pass per the task's own instructions).
DIM wp: BYTE
DIM chunk: STRING[120]
DIM i: INTEGER
chunk = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
ON ERROR GOTO 800
CREATE #wp, "/pipe/soloC": WRITE
PRINT #2, "soloW: created /pipe/soloC for WRITE, no reader will ever open it"
ON ERROR GOTO 900
FOR i = 1 TO 50
WRITE #wp, chunk
PRINT #2, "soloW: chunk "; i; " written OK"
NEXT i
PRINT #2, "soloW: all 50 chunks written with no error (unexpected if any cap/deadlock-detect exists)"
GOTO 999
800 PRINT #2, "soloW: CREATE failed, ERR="; ERR
GOTO 999
900 PRINT #2, "soloW: WRITE failed at chunk "; i; " ERR="; ERR
999 PRINT #2, "soloW: done"
END
