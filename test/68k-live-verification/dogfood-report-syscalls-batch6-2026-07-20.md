# Dogfood report: 68k syscall live-verification batch 6 (2026-07-20)

Continues batch 5. `F$DFork`, `F$DExec`, `F$DExit` — `batch6-01.a`,
using `childprg68k` (already on disk) as the debug target.

All three confirmed on the first live attempt, no hang: forked the
child suspended via `F$DFork` (a real register-frame buffer allocated
in the test's own data area), single-stepped it exactly one instruction
via `F$DExec` (no breakpoints), then killed it with `F$DExit`. The
parent correctly parked during `F$DExec` and resumed cleanly once the
single step completed — confirmed by checking the harness was still
responsive immediately afterward (`pd` returned normally).

## Running total

33 of the 68k syscall reference's calls are now `Live`-confirmed
(30 after batch 5 + 3 this batch).

## Files

- `test/68k-live-verification/batch6-01.a` — F$DFork/F$DExec/F$DExit
