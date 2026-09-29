# os9exec v4.1.1

A same-day fix to [v4.1.0](release-notes-v4.1.0.md), for two faults found in the published
release. Everything else in v4.1.0 stands; its notes are the full story.

**See it running:** [open a live OS-9 system in your browser](https://peacedudes.github.io/osk-freeware/try/).

## Fixed

- **A record-lock timeout could fire late.** A read blocked by another process's record lock,
  with an `SS_Ticks` limit set, gives up with `E$Lock` when the limit runs out. The waiter was
  only looked at again on the scheduler's rota, not when its deadline came, and on Windows it
  sometimes waited until the lock was released instead. Most visibly, the conformance disk
  run on Windows ended in "Please tell us" in about one run in three, with CONF68K t21 or
  t116 reporting `obs=000901`: nothing was wrong with the system under test. Measured on
  Windows 11: 6 runs of 20 before, 0 of 20 after. macOS and Linux were not seen to fail.
- **The conformance disk's verdict was unreadable at a terminal.** `run` and `tally` ended
  their lines with a carriage return alone, and SCF adds the line feed only for `I$WritLn`,
  so each line of the verdict overwrote the one before. This was true on real OS-9 hardware
  too. The report file, `RESULTS/report`, is unchanged.

## If you ran the conformance disk from v4.1.0

On Windows, a "Please tell us" whose only failures are t21 or t116 with `obs=000901` was this
fault, not your system. Nothing needs sending; running `conf68k.dsk` from this release gives
the right answer. On real OS-9 hardware, v4.1.0's disk gave correct results; only the last
screen was hard to read, and `RESULTS/report` was always right.

## Checking

The emulator's version is now V4.11 (`os9exec -ih`). The Windows check now runs the
conformance disk five times, from a fresh copy each time, and requires every run to pass: a
single run was what let the first fault through.
