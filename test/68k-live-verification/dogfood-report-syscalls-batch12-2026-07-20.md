# Dogfood report: 68k syscall live-verification batch 12 (2026-07-20)

Finishes the 18-item "Manual-only" list. `F$SSpd`, `F$Mem`, `F$SchBit`,
`F$AllBit`, `F$DelBit`, `F$Trans`, `F$UAcct` — `batch12-01.a`.

## Not a register-contract test — a "does this exist" test

Reading `funcdispatch.c` directly (before writing any test code, same
discipline as every other batch) showed all seven of these route to a
shared `OS9_F_UnImp` handler, not to a real implementation — unlike the
F$Send/F$Icpt findings (which install/accept but don't fully deliver),
these seven do nothing at all. `OS9_F_UnImp`'s own source: logs a debug
message and returns `E$UNKSVC` (208) unconditionally. So there's no
register contract to verify here — the test exists only to confirm live
that calling each one is a clean, safe error, not a crash or hang, and
that the error code matches source exactly.

## Confirmed cleanly

All seven returned `E$UNKSVC` (208) with the carry set, matching
`OS9_F_UnImp`'s source exactly. The host console additionally showed
`os9exec`'s own debug line for each (`# unimplemented F$SSpd called by
pid=6`, etc.) — direct confirmation the shared handler, not something
else, is what ran.

## A real bug in the test's own `check` subroutine, caught before the
## first live run

First draft tried to preserve the call-name string pointer (`a0`)
across a label-printing `I$Write` call by reading it back off the
stack afterward (`movem.l d0-d1,-(a7)` at entry, `movea.l (a7),a0`
later) — but the stack at that point held the just-pushed `d0`/`d1`,
not the original `a0` at all; nothing had actually saved it. Fixed by
copying `a0` into `a2` at entry via `move.l a0,a2` — an address-register
destination move is encoded as MOVEA and doesn't touch the condition
codes, so the still-pending carry flag from the syscall survives
untouched, and `a2` holds a genuinely preserved copy for later.

## Running total

55 of the 68k syscall reference's calls are now `Live`-confirmed (48 +
7 this batch). **The full 18-item Manual-only list is now resolved**:
11 got real register-contract verification (batches 9-11), 7 turned out
to not be implemented at all (this batch) — both outcomes are now
reflected in `68k/syscall-reference.md`, and the Manual-only section
itself has been replaced with a short "Not implemented on os9exec"
note for the seven.

## Files

- `test/68k-live-verification/batch12-01.a`
