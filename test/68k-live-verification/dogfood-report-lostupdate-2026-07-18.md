# Dogfood report: RBF automatic record-locking, lost-update race (2026-07-18)

Task: test OS-9/68000 RBF's automatic record-locking design intent
(`os9-systems-dev`'s `file-managers.md`, "Record Locking" section, case
1 -- firsthand design intent from the original Microware designer, per
project memory): a database-style read-modify-write cycle should be
race-safe under concurrent access with **zero explicit
application-level locking calls**, because a `Read` in update mode
automatically locks the record just read, and the following `Write` on
that same path automatically releases it.

Test shape (as specified, kept deliberately simple):

1. Create a file holding a single counter, starting at 0
   (`dogfood-lostupdate-init.bas`).
2. Two separate processes, each in a tight loop of N=300 iterations,
   race an **unprotected** read-modify-write against the same file: open
   for `UPDATE`, seek to 0, read the count, add 1, seek back to 0, write
   it back (`dogfood-lostupdate-incrementer.bas`) -- no `SS_Lock`
   anywhere in the program.
3. After both finish, check the final count
   (`dogfood-lostupdate-verify.bas`). If RBF's automatic locking works,
   final count = 2*N = 600 exactly. If not, some increments are lost
   and the final count is less than 600.

Target: OS-9/68000 via `os9exec` and `tools/os9repl.sh -r`, against a
**real RBF disk image** (`/h1/CLAUDETEST/counter.dat`), not a
host-native mount -- host-native mounts have no real record-locking
machinery underneath (per `using-os9exec-repl.md`), so this distinction
matters the same way it did for the earlier EOF-lock pass.

The racer keeps its path open across all 300 iterations (`OPEN` once,
loop `SEEK`/`GET`/`+1`/`SEEK`/`PUT`, `CLOSE` once at the end) rather than
re-opening per iteration -- a "long-lived connection" shape, chosen
because it's the natural way to exercise the per-path lock/unlock pair
repeatedly and needs no extra justification per the task's own
instructions. A fresh open/close per iteration would also have worked
(each iteration's `Read` immediately followed by `Write` on the same
path is what matters), just slower.

## Result

**RBF's automatic record-locking prevented every lost update, in both
of two independent full races.** Final count was exactly **600** (=
2*300) both times:

```
Race 1:
  claude: lostinc & lostinc &
  +6
  lostinc: done, 300 iterations, last local count seen=300
  +7
  claude: lostinc: done, 300 iterations, last local count seen=600
  ... lostver: final count=600

Race 2 (counter reset to 0 in between):
  claude: lostinc & lostinc & procs
  +6
  lostinc: done, 300 iterations, last local count seen=300
  +7
   Id PId Grp.Usr  Prior  MemSiz Sig S    CPU Time   Age Module & I/O
  lostinc: done, 300 iterations, last local count seen=600
  ... lostver: final count=600
```

**This validates case 1 of the design-intent explanation in
`file-managers.md`'s Record Locking section**, not contradicts it: a
program that does nothing but ordinary `Read`/`Write` calls in update
mode, with no `SS_Lock` anywhere, got real concurrency safety on this
project's RBF implementation for the classic lost-update pattern. This
is a meaningfully positive result given this project's own history --
the sibling EOF-lock dogfood pass (`dogfood-report-eoflock-2026-07-18.md`)
found the *other* record-locking case (case 2, the pipe-like
producer/consumer EOF lock) genuinely broken on the same RBF
implementation, so a clean result here was not a foregone conclusion;
this codebase does have real, live RBF locking bugs when they exist.

One honest caveat: the `procs` snapshot in race 2, sent immediately
after backgrounding both racers in the same combined shell command,
came back with an empty process list -- both racers had already
completed within roughly 0.1-0.4 real seconds, too fast for that
particular probe to catch them mid-flight. This doesn't undermine the
result (2*N was still an unforced, non-trivial correctness outcome the
scheduler could only produce by not corrupting the shared record), but
it does mean this pass has no *direct* evidence (like a process-table
snapshot) of genuine mid-race interleaving beyond the two processes
being launched as one combined backgrounded command and the arithmetic
coming out exactly right, twice, on a codebase that has already been
shown able to produce a wrong answer for a related but distinct locking
scenario. Weighed against that prior finding, getting exactly 2*N twice
in a row is treated here as solid evidence, not just a lucky
coincidence of full serialization.

## A real blocker found and worked around (not this test's bug)

Before either race could run, every attempt to compile or even
interactively type a BASIC09 program containing a plain numeric
assignment (`n = 1`, or a `TYPE` record field assignment) crashed
BASIC09 outright with `**** Can't install trap handler ****` followed by
`Error #000:216 (E_PNNF)`, dropping straight back to the OS-9 shell --
reproduced identically via `LOAD` from a host-authored file, via typing
the line directly into BASIC09's own line editor, and against a
previously-Live-verified file
(`tools/benchmarks/basic09-type-test.bas`) copied in fresh. This
initially looked like a severe, previously-undiscovered compiler crash
(a false lead recorded in this trio's own file headers: an early draft
with a `STRING` field in the counter's `TYPE` record was blamed, but
that was coincidental -- *any* numeric operation triggers the same
crash, not just `STRING` fields).

Root-caused via `os9exec`'s own `-d2 0x10A` syscall trace (`dbgSysCall |
dbgProcess | dbgTrapHandler`): BASIC09's first real numeric operation
calls `F$TLink` to link the `math` trap handler module (TRAP #15);
`install_traphandler`'s `link_load('math')` returned `E_PNNF` because
`math` was not actually resident, and BASIC09's own "can't install"
banner + `F$Exit` is its ordinary (if unfriendly) response to that --
not a crash bug in BASIC09 or `os9exec`'s CPU emulation at all. This
specific triage path is **already documented** in this skill's
`basic09/pack-and-runb.md`, item 1 of "`**** Can't install trap handler
****` triage" -- so the mechanism itself isn't new. What *is* new,
found and added to that file this pass: **the claude account's own
`/h0/startup` line, `load -s cio csl math`, does not actually make `cio`
or `math` resident** -- `mdir` after a completely fresh boot shows only
`csl` loaded (most likely pulled in as a dependency of `shell`/`tsmon`
themselves, not by that line) -- while a plain `load math` (no `-s`)
typed at the shell, alone, works every time, immediately. Reproduced
twice cleanly: `load -s math` alone silently fails to add `math` to
`mdir`; `load math` (same module, no flag) immediately succeeds. The
`-s` flag's own failure mode was not chased into `os9exec`'s source
further (out of scope for this pass, and `load` may be a genuine
Microware shell binary rather than an `os9exec`-internal command) --
flagged in `VERIFICATION-BACKLOG.md` for whoever picks up `os9exec`-side
RBF/shell work next. Practical workaround used for the rest of this
pass: `load math` manually before running any BASIC09 program that does
real arithmetic, once per fresh `os9exec` session.

## Harness / workflow notes

- `CREATE` on a file that already exists fails with `Error #000:218
  (E_CEF)` (Creating Existing File) -- expected, standard behavior, not
  a bug; `DEL` the counter file before each fresh `dogfood-lostupdate-init.bas`
  run when re-racing.
- The PACK/RunB workflow in `basic09/pack-and-runb.md` matched exactly:
  `LOAD` the racer, `SAVE lostinc >/dd/lostinc.src` (source, for the
  record), `PACK lostinc >/dd/CMDS/lostinc` (both print a spurious
  `Error #000:043` per that file's own documented gotcha -- the output
  files were verified present and correct with `ident`/`file` regardless
  of the message), then `bye` back to the shell and launch by bare name.
- The single-combined-command launch pattern
  (`lostinc & lostinc &`) from `using-os9exec-repl.md`'s "Launching two
  concurrent background processes reliably" section worked cleanly on
  the first try, both races -- no dropped/delayed/interleaved sends of
  the kind that section warns about for a *second, separate* `key` send.
- No wedged process, no Ctrl-C/Ctrl-E needed this pass -- both racers
  always ran to completion well within a second.

## Skill and backlog updates made as part of this pass

- `os9-systems-dev/file-managers.md`: Record Locking section's case 1
  now carries a `Live` confirmation of the design intent (this test),
  alongside the existing `Live` case-2 divergence (EOF lock).
- `os9-dev/references/basic09/pack-and-runb.md`: item 1 of the "Can't
  install trap handler" triage refined with the `/h0/startup`
  `load -s` silent-failure finding above.
- `VERIFICATION-BACKLOG.md`: this pass logged, plus the open `load -s`
  root-cause gap flagged for follow-up.
