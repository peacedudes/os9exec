PROCEDURE pipeprod3
! Dogfood task 2026-07-18: pipe IPC, testing common/ipc.md's "Pipes" section
! at larger scale than dogfood-pipes-producer.bas. Paired with
! dogfood-pipes-bulk-consumer.bas: `pipeprod3 ! pipecons3 &`.
!
! Claim under test: same 90-byte default-buffer claim, at 33x the volume --
! 100 separate WRITE calls of a 100-byte string each (10,100 payload bytes
! + 100 CRs before the sentinel), written in a tight loop with NO delay of
! its own. The paired consumer deliberately reads slowly (a busy-wait delay
! between every single READ), so for most of this run the producer has
! written far more than any 90-byte buffer could hold while the reader is
! still working through early records.
!
! Literal verified output (68k, os9exec, via tools/os9repl.sh, account
! "claude"): the consumer side printed
!   consumer3: read failed with ERR=211 after 101 successful reads, 10009 total data chars
! -- 100 chunks * 100 chars + 9-char sentinel = 10009, exactly matching.
! All data arrived intact with zero write errors reported by the producer
! (this file has no per-chunk ERR check by design, to avoid PRINT's own
! baud-rate-paced console output confounding the timing -- see
! dogfood-report-pipes-2026-07-18.md for why an earlier attempt at
! `procs`-polling mid-run to directly observe a blocked producer process
! didn't pan out, and what this run demonstrates instead).
DIM chunk: STRING[120]
DIM i: INTEGER
chunk = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
FOR i = 1 TO 100
WRITE #1, chunk
NEXT i
WRITE #1, "ENDOFDATA"
END
