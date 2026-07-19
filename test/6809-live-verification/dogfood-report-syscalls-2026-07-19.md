# Dogfood report: 6809 syscall register-contract regression tests (2026-07-19)

Closes most of `VERIFICATION-BACKLOG.md`'s "6809 (secondary priority)"
syscall-table gap: only `I$WritLn`/`F$Exit`/`F$Icpt`/`F$Send`/`F$ID` were
`Live` going in. This pass adds `F$Link`, `F$UnLink`, `F$Fork`, `F$Wait`,
`I$Open`, `I$Close` (both already exercised by the pre-existing
`dogfood-asm-line-counter.a` but never credited `Live` in the skill file —
fixed here too), `I$Read` (plain, not `I$ReadLn`), `I$Write` (plain, not
`I$WritLn`), and `F$Time`.

Three source files, designed to be re-run as a regression suite (not just
one-off probes): each prints one self-contained `PASS ...`/`FAIL ...`
line per assertion, so a future run just needs a `grep FAIL`.

- `syscall-childprg.a` — trivial helper module (`F$Exit` with B=77), not
  a test on its own. Assemble+install before the fork test.
- `syscall-flink-ffork-fwait.a` — `F$Link`/`F$UnLink`/`F$Fork`/`F$Wait`,
  8 assertions: success path for all four, plus three deliberate failure
  cases (`F$Wait` with no children, `F$Link` to a name that can never be
  resident, `F$Fork` of a name that can't resolve at all).
- `syscall-ftime-iread-iwrite.a` — `F$Time`/`I$Read`/`I$Write`, 6
  assertions: success path for all three plus round-trip content
  verification, EOF-past-end-of-file, `I$Open` on a nonexistent file, and
  `I$Read` on a never-opened path number. Pre-req: a `synctest.dat`
  placeholder file (any content/length) must exist first, since `I$Open`
  requires an existing path unlike `I$Create`.

All 14 assertions across both real test programs pass. Live output:

```
linkforktest:
PASS F$Wait-no-children correctly errored err=00226
PASS F$Link entry Y=58059
PASS F$UnLink
PASS F$Link bogus-name correctly errored err=00221
PASS F$Fork new PID=00004
PASS F$Wait PID matches forked PID
PASS F$Wait exit status is 77 as expected
PASS F$Fork bogus-name correctly errored err=00216

timeiotest:
PASS F$Time 00126 00007 00019 00001 00034 00051
PASS I$Write returned Y=00010
PASS I$Read content matches what was written
PASS I$Read past end-of-file correctly errors
PASS I$Open nonexistent-file correctly errored err=00216
PASS I$Read bogus-path correctly errored err=00201
```

## New facts learned, not previously in the skill

- **`F$Time`'s year byte is `year - 1900`**, not a raw counter or 2-digit
  value: byte 0 read `126` against a known guest date of 2026-07-19
  (126+1900=2026). Previously the doc only said "year/month/day/hour/
  min/sec" with no encoding specified.
- **`I$Write` does return Y** (the actual byte count written, `Y=10` for
  a 10-byte write) — previously undocumented (`I$Read`'s row explicitly
  says "Returns Y=actual bytes"; `I$Write`'s row said nothing about a
  return value at all).
- **`F$Link` and `F$Fork` fail differently against an unresolvable name,
  and the distinction is meaningful**: `F$Link` searches only the
  resident in-memory module directory and fails `E$MNF` (221, "module
  not found") against a name that's never been loaded; `F$Fork` does its
  own load-from-pathlist internally (same as `F$Load`) and instead fails
  `E$PNNF` (216, "path name not found") since it's really a file lookup
  underneath. A caller distinguishing "not loaded yet" from "doesn't
  exist at all" needs to know which of the two error codes it's looking
  at, and which call is even capable of telling them apart.
- **`F$Wait` with no children fails `E$NoChld` (226)** — the doc already
  said "error if no children" but didn't have the code.
- **Confirms `I$Open` requires an existing file** (fails `E$PNNF`/216
  against a nonexistent name) — it does not create-on-write the way some
  Unix `open()` flag combinations do; `I$Create` is the separate call for
  that.
- **A bogus/never-opened path number fails `E$BPNum` (201, "illegal path
  number")** on `I$Read` — confirms path numbers are validated, not just
  trusted.

## Bugs found in the tests themselves while writing them (not os9exec/OS-9 bugs)

Two real register-clobber mistakes, both the same underlying lesson —
**a syscall's return register must be copied into memory *immediately*
after the call, before any helper/print routine runs**, since those
helpers reuse the same registers (`B` as a byte counter in `copys`, `Y`
as a scratch pointer via `leay`) and will silently destroy an unsaved
return value:

1. First live run of the fork/wait test printed `exit status wrong,
   got=00000` even though the child really did exit with B=77 — the PID
   check (which only reads `A`) ran fine, but by the time the status
   check (`cmpb #77`) ran, three `copys`/`crwrite` calls for
   intermediate PASS messages had already clobbered `B`. Fixed by
   `stb EXITSTAT,u` as the very first instruction after the `F$Wait`
   `swi2`, before any print logic.
2. First live run of the I/O test printed `I$Write returned Y=15616` —
   garbage, since `Y` (the real return value) was never captured before
   a `leay IWPAS,pcr` (loading the pass-message pointer) overwrote it.
   Fixed by `tfr y,d` / `std VALUE,u` immediately after the success
   branch is entered, before anything else touches `Y`.

Also hit again, for the third time across this project's 6809 assembly
dogfooding: `asm`/MIA's 8-character label truncation silently colliding
(several `PASS`/`FAIL` message-length label pairs like `flinkpass`/
`flinkpassl` both truncate to `flinkpas`) and `bsr`/`bra`'s ±127-byte
range being easy to exceed once a program has this many call sites to
shared helpers. Both are already documented gotchas (see
`6809/assembly-and-tools.md` and the line-counter test's own header) that
should have been applied up front and weren't — noting again since it
clearly bears repeating for whoever writes the next one of these.

## Method notes for whoever extends this suite

- **Uploading source this size**: typing more than a few lines via
  `nitros9repl.sh key`/`tee` is impractical. Used ToolShed (`os9 copy -l`)
  to inject `.a` files directly into the disk image host-side, same
  technique as the GFX2 package install
  (`dogfood-gfx2-install-notes.md`) — XRoar must be **stopped** first
  (`nitros9repl.sh stop`), extract the partition
  (`dd if=68IDE.ide of=partition.img bs=512 skip=632`), `del`+`copy -l`
  (not `-r`, see that same file's documented segment-corruption bug),
  write the partition back (`dd ... seek=632 conv=notrunc`), then
  `nitros9repl.sh start` again. The `.ide` file persists the change
  across restarts.
- **`asm`'s output lands in `CMDS`** (the execution directory), not the
  current data directory — `ident /dd/CMDS/<name>` to inspect it
  (bare `ident <name>` / `ident CMDS/<name>` both 216'd in this session;
  the full `/dd/CMDS/...` path is what worked).
