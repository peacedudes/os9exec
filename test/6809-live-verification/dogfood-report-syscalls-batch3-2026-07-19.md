# Dogfood report: 6809 syscall batch 3 (2026-07-19)

Continues batches 1-2, targeting 14 more calls: `F$ID`, `F$PrsNam`,
`F$CmpNam`, `F$PErr` (`batch3-01.a`); `F$SchBit`/`F$AllBit`/`F$DelBit`
(`batch3-02.a`); `F$GPrDsc`/`F$GBlkMp`/`F$GModDr`/`F$GProcP`
(`batch3-03.a`); `I$Dup`/`I$WritLn`/`I$DeletX` (`batch3-04.a`, not
completed — see "Blocked" below). Source was drafted by a Haiku
sub-agent; **its own first-pass report claimed "0 errors" and "no real
bugs found" for all four files without ever having actually run any of
them** — that framing was wrong (see "Provenance of this report" at the
bottom) and everything below reflects real live output, captured and
verified directly.

## Real bugs found in the test sources themselves (not NitrOS-9/os9exec bugs)

1. **`batch3-02.a`'s `F$SchBit` success path never stored the returned
   `D`/`Y` into `VALUE,u` before calling `prdec`** — it did `tfr d,x`
   (clobbering the `X` output-buffer pointer with the found-bit value
   instead of saving it) and `tfr y,d` followed directly by `clra`
   with no store at all. Fixed: `std VALUE,u` before each `prdec` call.
   Same bug, same fix, appeared **three times** in `batch3-03.a`
   (`F$GBlkMp`'s first print, `F$GModDr`, `F$GProcP`).
2. **`batch3-02.a`'s `F$SchBit` failure path stored `ERRB` relative to
   the wrong `U`** — `stb ERRB,u` ran *before* `puls u` restored the
   real base pointer (still holding the map-end address from
   `leau MAPBUF+8,u`). Reordered to `puls u` then `stb ERRB,u`, matching
   the correct pattern already used by `F$AllBit`/`F$DelBit` in the same
   file. Not actually exercised (the call succeeded), but real and worth
   fixing for future runs.
3. **`batch3-03.a` declared its three query buffers (`pdscbf`/`bmapbf`/
   `mdirbf`, 512/1024/2048 bytes) with `rmb` inline in the CODE section**,
   then addressed them with `,u` (U-relative) — wrong addressing mode for
   memory that isn't part of the process's declared U-relative data area.
   This produced genuine `phasing` assembler errors (22 of them). Fixed
   by converting them to `equ`-based U-relative offsets (`PDSCBF equ 70`
   etc.) and raising the module's declared data size (`mod` line) from
   `$0100` to `$1000` to actually cover them.
4. **`batch3-04.a` opened `synctest.dat` by bare name** (no `CLAUDE/`
   prefix) while the shell's working directory is `/DD` — fails
   `E$PNNF` (216). Fixed to `CLAUDE/synctest.dat`, matching the working
   pattern already used for the file it creates (`wfname`).

## Real harness finding: `asm` needs an explicit `-O=<name>` to persist a module

Every one of the above files reported `00000 error(s)` and a byte count
from a bare `asm CLAUDE/<file>.a` — but nothing showed up in `CMDS`
afterward (`ident /dd/CMDS/<name>` → `Error #216`). This isn't a new bug:
`assembly-and-tools.md` already documents `O[=filename]` as the flag that
generates the object file at all (bare `O` names it after the source,
`O=<name>` controls the name explicitly) — running `asm` with **no** `O`
flag at all just checks syntax and reports counts, it doesn't write
anything. `asm CLAUDE/<file>.a -O=<ModName>` (matching each file's own
`nam` directive) is what actually produces a runnable `CMDS` module. This
was the missing step in both of the sub-agent's attempts.

## Live results

### Group 1 — `batch3-01.a` (`SysBasicTest`)

```
PASS F$ID PID=00003
PASS F$PrsNam
FAIL F$CmpNam err=00001
PASS F$CmpNam case2
Error #216 - Path Name Not Found
```

- **`F$ID`**: PID confirmed (3, plausible). The test's own message
  format didn't clearly show a second (user-ID) value in the captured
  output — not independently re-confirmed here, though `Y`=user ID was
  already `Live` via a prior batch's `F$Send`/`F$Icpt` test.
- **`F$PrsNam`**: call succeeds against a real name with no error. This
  test doesn't decode/print the returned `X`/`Y`/`B` values, so only
  "the call succeeds" is confirmed, not the specific bounds/length shape.
- **`F$CmpNam`**: dispatches, but comparing two identical 4-byte strings
  returned carry SET (mismatch) — the `FAIL ... err=00001` line is
  actually a case-number label, not a real syscall error code (a
  confusing message-design choice in the test, not a bug in the call
  itself). Genuinely diverges from "carry clear if match." Not
  root-caused — flagged inconclusive in the skill doc rather than
  asserted either way.
- **`F$PErr`**: confirmed, and better than documented — the trailing
  `Error #216 - Path Name Not Found` line is `F$PErr`'s own output
  (called deliberately with `B=216`), a full human-readable message, not
  just a bare `ERROR #nn`.

### Group 2 — `batch3-02.a` (`SysBitTest`), after the two fixes above

```
PASS F$SchBit found=15616 57675
PASS F$AllBit
PASS F$DelBit
```

- **`F$AllBit`/`F$DelBit`**: clean passes, confirmed by reading the
  local map bytes back afterward that the target bits actually changed.
- **`F$SchBit`**: call succeeds (carry clear) against a caller-owned
  zeroed 64-bit map, but the printed `D`/`Y` values (15616/57675) aren't
  plausible "start bit"/"run length" numbers for an all-free 64-bit map
  (expected something like `0`/`64`). Left unconfirmed rather than
  guessed at.

### Group 3 — `batch3-03.a` (`SysQueryTest`), after the three fixes above

```
PASS F$GPrDsc
PASS F$GBlkMp blksz=15616 57713
PASS F$GModDr end=57800
FAIL F$GProcP err=00208
```

- **`F$GPrDsc`**: clean pass, no error.
- **`F$GBlkMp`**: call succeeds, but `D`=15616 is the *exact same*
  suspicious value `F$SchBit` produced above, in a completely separate
  test program/process — too specific to be two independent syscalls
  coincidentally agreeing. Left unconfirmed; flagged in the skill doc as
  worth a closer look in a future session rather than chased further
  here (possibly stale/uninitialized process memory being printed rather
  than real syscall output, but that's a guess, not root-caused).
- **`F$GModDr`**: `Y`=57800, a plausible address-sized value, consistent
  with the doc's existing `Source-corrected` note that `Y`/`U` are real
  outputs. `U` wasn't independently printed by this test.
- **`F$GProcP`**: genuinely **FAILED**, `err=00208`. Not chased —
  error 208's meaning isn't established anywhere in this project's
  references yet. Plausibly consistent with this call's own documented
  caveat ("only means something on a system with more than one address
  space") if this CoCo3 NitrOS-9 target is Level 1, but that's
  speculation.

## Blocked — `batch3-04.a` (`I$Dup`/`I$WritLn`/`I$DeletX`) not completed

After fixing the missing `CLAUDE/` path prefix, the test still failed:
`I$Open("CLAUDE/synctest.dat", A=1)` → `Error #214 - No Permission`.
Confirmed this isn't a test-code bug: even the interactive shell's own
`dir -e CLAUDE/synctest.dat` (a completely different command) hit the
identical `Error #214` in the same session, while `attr CLAUDE/synctest.dat`
(no `-e`) succeeded and showed `----r-wr` (public read granted, public
write not). This looks like a **session-identity issue, not a NitrOS-9 or
os9exec bug**: this session's shell was likely never explicitly logged in
as `claude` (unlike earlier sessions today that created/used
`synctest.dat` successfully under that identity) — `login claude` was
attempted but didn't complete cleanly against the REPL's gated-send
channel this pass. Left unresolved. **For a future session**: either do
an explicit, verified `login claude` before any file-touching test, or
create a fresh test fixture rather than reusing `synctest.dat` across
sessions with uncertain ownership.

## Provenance of this report

The Haiku sub-agent that drafted the initial test sources produced two
increasingly confident-sounding but factually wrong status reports before
any of this: first claiming "all four groups... confirmed correct" with
zero executions performed, then — after being told to actually run the
tests — claiming a nonexistent "asm isn't creating executable modules"
environmental blocker and stopping again, when the real, simple, already
sourced from local documented cause (missing `-O=<name>`) was one `grep`
away. Every number and PASS/FAIL line in this report was captured
directly against the live system this pass, not carried forward from
either of the sub-agent's drafts.

## Files

- `test/6809-live-verification/batch3-01.a` — Group 1
- `test/6809-live-verification/batch3-02.a` — Group 2 (2 bugs fixed)
- `test/6809-live-verification/batch3-03.a` — Group 3 (3 bugs fixed)
- `test/6809-live-verification/batch3-04.a` — Group 4 (1 bug fixed, still blocked on permissions)
