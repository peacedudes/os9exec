# Dogfood report: 68k syscall live-verification batch 4 (2026-07-20)

Continues batches 1-3. `F$SPrior`, `F$CRC`, `F$SetCRC`, `I$GetStt`
(SS.Size) — `batch4-01.a`.

## Two self-inflicted register-clobber bugs, caught by cross-checking against real, known values

Both `F$CRC` and `I$GetStt`'s success-path prints called `I$Write` (to
print a "PASS ..." message) *before* saving the syscall's real result
out of the register it returned in — the message-write's own `d1`
usage silently overwrote the value about to be printed. Caught only
because the printed numbers were checked against independently-known
truth rather than just trusted as "a number came out, call it good":

- **`F$CRC`** first printed `result=18` — exactly the character count of
  `"PASS F$CRC result="`, not a CRC at all. Fixed by saving `d1`
  (the real accumulator) to `d6` before the message `I$Write`, printing
  `d6`. Now prints a real, plausible nonzero CRC value.
- **`I$GetStt`(SS.Size)** first printed `SS.Siz=21` against a file
  independently confirmed via `dump`/`dir -e` to be exactly 10 bytes —
  again the character count of the pass message
  (`"PASS I$GetStt SS.Siz="`, 21 chars), not the real size. Root-caused
  properly this time (not just patched and re-tested): traced
  `OS9_I_GetStt` → `usrpath_getstat` → `filestuff.c`'s `SS_Size` dispatch
  case, which passes `&d2` as the output pointer — **the real result
  comes back in `d2.l`, not `d1`**, a previously-undocumented register
  convention (the skill only ever said "per code"). Fixed to read `d2`;
  reprinted result matched the real file size exactly (10).

Both are genuine mistakes in this test's own code, not `os9exec`
issues — flagged prominently here specifically as a reminder of the
methodology lesson: **verify a "PASS" result against an independently-known
true value whenever one is available** (a real file size via `dir -e`,
a real byte count via `dump`), not just trust that "no error + some
number printed" means the number is meaningful. The `F$CmpNam`
correction earlier this session was the same lesson learned the hard
way; this batch is where it was actually applied before publishing.

## Confirmed cleanly, no issues

- **`F$SPrior`**: sets the caller's own priority, no error.
- **`F$SetCRC`**: pointed at a code address (not a real module header)
  — correctly rejected with `E$BMID` (205), confirming the call
  validates its input. Success path (a real, valid module image) not
  exercised this pass.

## Running total

27 of the 68k syscall reference's calls are now `Live`-confirmed
(23 after batch 3 + 4 this batch).

## Files

- `test/68k-live-verification/batch4-01.a` — F$SPrior/F$CRC/F$SetCRC/I$GetStt(SS.Size)
