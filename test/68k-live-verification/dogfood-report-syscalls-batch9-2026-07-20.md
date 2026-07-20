# Dogfood report: 68k syscall live-verification batch 9 (2026-07-20)

Starts on the 18 "Manual-only" calls (register detail never reproduced
in the skill, only attested by name) now that the detailed tier is
done. `F$Julian`/`F$Gregor`, `F$PrsNam`, `F$PErr` — `batch9-01.a`.

## Confirmed cleanly

- **`F$Julian`/`F$Gregor` round-trip**: `os9exec`'s source shows the
  date is packed as `(year:16)(month:8)(day:8)` in `d1.l`, not
  decimal-digit "yyyymmdd" despite the doc comment's naming — worth
  catching before writing the test, not after a confusing live failure.
  Fed noon on 2026-07-20 into `F$Julian`, then fed its own output
  straight into `F$Gregor`: exact match on both `d0`/`d1`. This proves
  the register contract without needing to independently verify
  `j_date()`'s internal epoch.
- **`F$PrsNam`**: tested both without a leading `/` ("hello/world" →
  terminator=`/`, length=5) and with one ("/etc" → a0 advances exactly
  1 byte past the slash, terminator=NUL, length=3). The second case is
  the exact code path that once had a double-evaluated `++p` bug
  (`os9-shell-rejects-single-char-redirect-target`, fixed earlier this
  project) — still correct.
- **`F$PErr`**: prints directly to the emulator's own console via
  `upe_printf`, not through `I$Write` — same channel as the earlier
  uncontrolled `F$Chain` error message, so it shows up in the host
  capture the same way. Confirmed producing the expected
  `Error #000:216 (E$PNNF) Path Name Not Found` line.

## A self-inflicted bug caught before it corrupted the test

First draft of Stage 1 printed "PASS F$Julian" and two labeled results
via `I$Write` calls placed *between* the `F$Julian` call and the values
being printed — but `I$Write` itself sets `d0`=path number and
`d1`=length, silently overwriting the real `F$Julian` results before
they were ever read. Exactly the class of bug caught in batch 4
(`F$CRC`/`I$GetStt`). Fixed by saving `d0`/`d1` into scratch registers
immediately on return from the syscall, before any label-printing
`I$Write`. Same fix applied in `F$PrsNam`'s two stages.

## Running total

41 of the 68k syscall reference's calls are now `Live`-confirmed (37
detailed-tier + 4 this batch, out of what was previously the 18-item
Manual-only list).

## Files

- `test/68k-live-verification/batch9-01.a`
