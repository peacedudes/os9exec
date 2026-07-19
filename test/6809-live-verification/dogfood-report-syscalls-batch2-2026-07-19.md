# Dogfood report: 6809 syscall/asm-option batch 2 (2026-07-19)

Companion to `dogfood-report-syscalls-2026-07-19.md` (same day, earlier
batch). Continues clearing `VERIFICATION-BACKLOG.md`'s syscall list:
`I$MakDir`, `I$ChgDir`, `I$Create`, `I$Delete`, `I$Seek`, `I$GetStt`,
`F$Mem`, `F$Sleep`, `F$SPrior`, `F$CRC`. Also spot-checks `asm`'s
remaining command-line option toggles (`D`, `W`, `N`, `I`, `U`).

## Syscalls — all PASS except I$GetStt's return convention

`syscall-imakdir-ichgdir-icreate-idelete.a`: all 4 pass —
`I$MakDir(B=attributes,X=pathlist)` creates a directory,
`I$ChgDir(A=1,X=pathlist)` changes the data directory (confirmed by a
bare-name `I$Create` succeeding relative to it), `I$Create(A=mode,
B=attributes,X=pathlist)` creates+opens a new file, `I$Delete(X=pathlist)`
removes it. Register contracts all confirmed as documented.

`syscall-fmem-fsleep-fsprior-fcrc.a`: all 4 pass.
`F$Mem(D=0)` (query) returned a plausible upper-bound address in Y.
`F$Sleep(X=50)` returned with `remaining=23`, not `0` — **worth noting
plainly, not asserted as a bug**: the call didn't error and did pause
execution, but it didn't report having consumed the full requested tick
count either. Not chased further; a real timing/scheduling explanation is
plausible (e.g. tick granularity or an early wake condition), but this
diverges from a naive "returns 0 when the full duration elapsed"
assumption and is worth remembering if `F$Sleep` timing precision ever
matters. `F$SPrior(A=ownPID via F$ID, B=150)` accepted cleanly.
`F$CRC(X=start,Y=count,U=accumulator init $FFFFFF)` completed without
error and produced a 3-byte result; not independently verified against a
reference CRC implementation (no known-answer test data available), so
this confirms the *call* works, not the specific checksum value.

`syscall-iseek.a`: **confirms the X:U 32-bit position convention**
`syscalls-and-module-format.md` already flagged as `Source-checked` (from
NitrOS-9's own `rbf.asm`) rather than the manual's "D or X:D" claim —
seeking to offset 5 in a known 10-byte file (`synctest.dat`,
`"HELLOWORLD"`) via `X=0,U=5` and reading 5 bytes back landed exactly on
`"WORLD"`.

`syscall-igetstt.a`: `I$GetStt(A=path,B=6 SS.EOF)` accepted cleanly
(confirms the call is valid before EOF). **`I$GetStt(A=path,B=2 SS.SIZ)`
is accepted with no error, but the guessed X:U return convention for the
size value did not check out** — it returned `X:U=44:10` against a known
10-byte file, and 44 makes no sense as a high word for that size. Read
this as "the call succeeds, the return-value register convention remains
genuinely unconfirmed," not as a confirmed (wrong) answer — X may simply
be leftover/untouched by this GETSTAT function, meaning the size might
come back some other way (a different register pair, or not at all for
this function code) that this pass didn't isolate.

## Real bug found while writing `syscall-iseek.a`, not a NitrOS-9 finding

`U` is both this program's own U-relative data-area base pointer *and*
the register `I$Seek`'s calling convention requires for the position's
low word — `ldu #5` silently destroyed the base pointer, corrupting
every subsequent `,u`-relative reference (`PATHNUM,u` included) for the
rest of the program, producing a spurious `E$BPNum` (201) on the very
next call. Fixed with `pshs u` / `puls u` bracketing the `swi2` (`PULS`
doesn't affect `CC`, so the carry/error result survives the restore
untouched). **General lesson for any future 6809 test program**: any
syscall whose calling convention uses `X` or `U` for real data (not just
`A`/`B`/`D`) needs this same save/restore discipline if the program also
uses that register as its own addressing base — a third instance of the
register-clobber class of bug that's now bitten this test suite multiple
times (see the syscall batch 1 report's header for the first two).

## `asm` option toggles

Building on batch 1's `L`/`S` (confirmed) and `G` (inconclusive):

- **`D<num>` (page depth) confirmed.** `asm childprg.a L D5` produced 42
  separate numbered pages (vs. ~2 at the `D66` default) for the same
  13-line source — exactly the documented "lines per page, headers/blanks
  included" behavior.
- **`W<num>` (max listing width) confirmed.** `asm childprg.a L W20`
  truncated every listing line, including the page-header line itself, to
  20 characters.
- **`N` (narrow/non-columnized format) confirmed.** Output drops the
  fixed-width address/bytes/label columns entirely — source and object
  bytes run together on one line per statement instead of aligning into
  columns.
- **`I` — real finding, looks like a genuine `asm` bug, not just an
  undocumented mode.** Every listing line gets an `ASM:` prefix (support
  for the "interactive mode" theory this file already carried), but **the
  assembled module header comes out wrong** — `mod eom,nm,...`'s encoded
  bytes read `87CD3103` under `I`, vs. the correct `87CD001D` every other
  invocation of the identical source produces, with the trailing CRC also
  different (and implausible: `3103` as part of a module-size-derived
  field doesn't correspond to this program's real 29-byte size). Assembly
  still reported `00000 error(s)`. **Practical warning: don't use `I` when
  assembling a file passed as a normal command-line argument — it may
  silently produce a corrupted module while reporting success.** Not
  investigated further (matches the file-passed-as-argument vs.
  presumed-interactive-stdin-input theory, but that's speculation, not
  confirmed).
- **`U` — no observable effect** on this source; output was byte-identical
  to the plain default listing (correct header, correct CRC). Genuinely
  inconclusive whether `U` does nothing for this kind of source or has an
  effect this test didn't trigger.
- `C`/`E`/`F` remain untested (need, respectively, a source with
  conditional-assembly blocks, a source with a real error to compare
  full-message-vs-flag-only display, and a way to observe form-feed
  control characters through this text channel — none attempted this
  pass, reasonable stopping point for a low-priority item).

## Harness notes

Same ToolShed batch-injection recipe as batch 1 (stop XRoar, `dd` extract
partition at `bs=512 skip=632`, `os9 del`+`copy -l` per file, `dd`
re-inject at `seek=632 conv=notrunc`, restart). Hit the `bra`-out-of-range
and `lda #'x'`-character-literal issues from batch 1 again in three of
these four new files — same fixes (`lbra`, hex literals like `#$20`/`#$3A`
instead of quoted chars) — worth internalizing as defaults for any new
6809 test program rather than re-discovering per file.
