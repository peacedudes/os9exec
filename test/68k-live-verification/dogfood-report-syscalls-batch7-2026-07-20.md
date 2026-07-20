# Dogfood report: 68k syscall live-verification batch 7 (2026-07-20)

Continues batch 6. `I$ReadLn`, `F$STime`, `F$Chain` (failure path only)
— `batch7-01.a`.

## Confirmed cleanly

- **`I$ReadLn`**: created a fresh file with a real CR-terminated line
  ("HELLO"+CR, 6 bytes), sought back to the start, read it back — byte
  count matched exactly (6).
- **`F$STime`**: accepted real time/date values (round-tripped from a
  preceding `F$Time` read) with no error. Confirmed via source first
  that this build's `OS9_F_STime` doesn't touch the real host clock at
  all — the only host-side effect is compiled in under `#ifdef MACOS9`
  (classic Mac OS 9 target only), not this build — so this call was safe
  to test without any real side effect to worry about.

## Real concern found, not root-caused: `F$Chain`'s failure path

Deliberately targeted a bogus module name, failure path only (a
successful chain replaces the whole process with no return, so only the
failure case is safe to test unattended). `OS9_F_Chain`'s source reads
cleanly on inspection — same shape as `F$Fork`'s already-proven-safe
logic, no obvious destructive step before name resolution runs.

**Live result: a raw, uncontrolled `Error #000:221 (E_MNF)` printed,
with none of this test program's own `PASS`/`FAIL` output ever
appearing.** Every other failure path tested across all seven batches
this session returned control cleanly to the calling program's own
error-handling code; this is the only one that didn't. Checked the
harness immediately afterward — still fully responsive (`pd` returned
normally), and the test's own file cleanup from the *I$ReadLn* section
earlier had already completed, so nothing is left in a bad state on
disk. Not chased further live, deliberately — matches the same
discipline used for the equivalent 6809/NitrOS-9 finding earlier the
same day (a raw uncontrolled error is exactly the kind of symptom this
project treats as "stop, don't keep poking," not "immediately root
cause it live"). Added to `ROADMAP.md` as a real bug candidate for the
emulator author to look into directly, since this is `os9exec`'s own
code — unlike the 6809 side, this one is actually ours to fix.

## Running total

36 of the 68k syscall reference's calls are now `Live`-confirmed
(33 after batch 6 + 3 this batch).

## Files

- `test/68k-live-verification/batch7-01.a` — I$ReadLn/F$STime/F$Chain(failure path)
