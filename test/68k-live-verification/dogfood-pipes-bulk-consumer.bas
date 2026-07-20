PROCEDURE pipecons3
! Dogfood task 2026-07-18: pipe IPC, larger-scale companion to
! dogfood-pipes-consumer.bas. Paired with dogfood-pipes-bulk-producer.bas:
! `pipeprod3 ! pipecons3 &`. See that file's header for the claim and the
! literal verified result (101 reads, 10009 total data chars, clean
! ERR=211 EOF on the 101st READ attempt).
!
! This consumer inserts a busy-wait delay BEFORE EVERY READ (not just the
! first), so across the whole run there's an extended window where the
! producer -- which writes all 10KB in a tight loop with no delay of its
! own -- has necessarily written far ahead of what's been drained. Intent
! was to catch the producer in a blocked/waiting state via `procs` polled
! from a separate REPL command mid-run; in practice the REPL round-trip
! latency dominated and no run was ever caught mid-flight (see the dogfood
! report) -- `procs` itself also rendered zero process rows on this
! harness even at rest, a separate unresolved oddity, not chased further.
! Kept as a live-verification artifact for the volume/integrity result,
! which stands on its own regardless.
DIM line: STRING[200]
DIM n, totalchars, k: INTEGER
n = 0
totalchars = 0
ON ERROR GOTO 900
100 FOR k = 1 TO 250000
NEXT k
READ #0, line
n = n + 1
totalchars = totalchars + LEN(line)
GOTO 100
900 PRINT #2, "consumer3: read failed with ERR="; ERR; " after "; n; " successful reads, "; totalchars; " total data chars"
END
