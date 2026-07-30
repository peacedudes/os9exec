# Our runs

This file records what *this project's own* NitrOS-9 (6809) system did when
it ran CONF6809. It is a candidate observation from one reimplementation,
recorded here so a reader can compare it against their own hardware or
emulator -- **it is not a standard, and this suite's own PASS/FAIL verdicts
do not depend on it.** Every test's actual pass/fail criterion is the
manual citation in `DOCS/claims.md`, not this file. If your system disagrees
with what is recorded below, that disagreement is itself useful data --
please send it back (see `text/readme`).

os9exec (this repository's 68k emulator and OS-9/68000 kernel
reimplementation) cannot contribute an observation here: CONF6809 is a
6809 disk built for and run under real OS-9/6809 (NitrOS-9), and os9exec
does not run 6809 code. Its only role in this project is building the
image (`build-image.sh` uses it as a host-side RBF formatting/copy tool,
the same way any host program could) and reading `h0`, its own separate
68k system disk, for its own unrelated 68k conformance testing.

## System observed

- NitrOS-9/6809 Level 2 V3.3.0 on an emulated Tandy Color Computer 3
  (EOU `eou_ide-v0.3-6809-xroar-dw-becker` disk image).
- Emulator: XRoar 1.12.1, headless, full speed (`-no-ratelimit`).
- Bridge: DriveWire, `drivewire-cli` (`drpitre/drivewire`), tunneled over
  XRoar's becker port. **Not stock upstream `main`**: the build used carries
  a one-line queue-confinement fix for a server crash that upstream `main`
  still has (offered as DrPitre/DriveWire PR #8, open at the time of this
  run). That fix is host-side only -- it changes which dispatch queue the
  server's own callbacks run on, and touches nothing the guest sees -- so it
  affects whether the harness stays up, not what OS-9 does. The conformance
  image itself is served to the guest as a second virtual disk (`--disk0`,
  appearing as `/X0`) alongside the guest's own real boot disk (`/DD`) --
  see `tools/selfhost6809/run-on-nitros9.sh`.
- Login: `USER1`, this disk's ordinary non-super account (flat ID 1; ID 0
  is the only superuser on this system -- see the owner-ID paragraph in
  `text/readme`).

## Prebuilt run (`CMDS/`, shipped as-is)

```
RUN prebuilt

         User                     Mem Stack
Id  PId Number  Pty Age Sts Signl Siz  Ptr   Primary Module
--- --- ------- --- --- --- ----- --- ----- ----------------
  3   9     1   128 131 $80    0   31 $33E2 Shell
  4   3     1   128 128 $80    0   31 $1EF3 Procs
  9   7     1   128 131 $80    0   31 $2EE2 Shell
RESULT t01 PASS  obs=00216 exp=00216  I$Open of an absent pathlist reports E$PNNF
RESULT t02 PASS  obs=32768 exp=32768  INTEGER overflow on 6809 wraps modulo 65536 (32767+1=-32768)
RESULT t03 PASS  obs=00211 exp=00211  I$Read at end of file reports E$EOF
RESULT t04 PASS  obs=00203 exp=00203  writing to a path opened read-only reports E$BMode
RESULT t05 PASS  obs=00214 exp=00214  I$Open without permission reports E$FNA
RESULT t06 PASS  obs=00077 exp=00077  I$Seek honors the X:U 32-bit position convention
RESULT t07 PASS  obs=00045 exp=00045  INTEGER divide by zero traps into BASIC09's own error 45
RESULT t08 PASS  obs=00054 exp=00054  a write past a seek beyond EOF extends the file
RESULT t09 FAIL  obs=00194 exp=00200  concurrent read-modify-write on one record loses no update
RESULT t10 PASS  obs=00007 exp=00007  a reader waits at the edge for an update-mode producer
RESULT t11 PASS  obs=00007 exp=00007  a write-only producer holds the EOF lock as update does
RESULT t12 PASS  obs=00001 exp=00001  a second write-only writer waits at the locked end
```

`tally`: `CONF6809 totals: PASS=11 FAIL=1 SKIP=0 ERROR=0`

## Rebuilt run (`rebuild`: SRC/ reassembled with this system's own `asm`,
repacked with its own `basic09`, run from `REBUILT/`)

```
RUN rebuilt

         User                     Mem Stack
Id  PId Number  Pty Age Sts Signl Siz  Ptr   Primary Module
--- --- ------- --- --- --- ----- --- ----- ----------------
  3   9     1   128 131 $80    0   31 $33E2 Shell
  4   3     1   128 131 $80    0   31 $1EF3 Procs
  9   7     1   128 131 $80    0   31 $2EE2 Shell
RESULT t01 PASS  obs=00216 exp=00216  I$Open of an absent pathlist reports E$PNNF
RESULT t02 PASS  obs=32768 exp=32768  INTEGER overflow on 6809 wraps modulo 65536 (32767+1=-32768)
RESULT t03 PASS  obs=00211 exp=00211  I$Read at end of file reports E$EOF
RESULT t04 PASS  obs=00203 exp=00203  writing to a path opened read-only reports E$BMode
RESULT t05 PASS  obs=00214 exp=00214  I$Open without permission reports E$FNA
RESULT t06 PASS  obs=00077 exp=00077  I$Seek honors the X:U 32-bit position convention
RESULT t07 PASS  obs=00045 exp=00045  INTEGER divide by zero traps into BASIC09's own error 45
RESULT t08 PASS  obs=00054 exp=00054  a write past a seek beyond EOF extends the file
RESULT t09 FAIL  obs=00181 exp=00200  concurrent read-modify-write on one record loses no update
RESULT t10 PASS  obs=00007 exp=00007  a reader waits at the edge for an update-mode producer
RESULT t11 PASS  obs=00007 exp=00007  a write-only producer holds the EOF lock as update does
RESULT t12 PASS  obs=00001 exp=00001  a second write-only writer waits at the locked end
```

`rebuild`'s own reassembly step: 5 `asm` invocations (the assembly tests),
`00000 error(s)` each, with 2, 11, 10, 6 and 10 warnings respectively --
harmless, and the same counts every run; 12 `basic09` `PACK`s (the BASIC09
tests, their four helper processes, and `tally`), all successful. Each
`PACK` gets its **own** `basic09` invocation. They shared one before, which
stopped working the moment Phase 2 took the count from 4 to 10: the workspace
filled, `LOAD` began truncating procedures part-way -- surfacing first as ten
`Error #074 -- Undefined Line Number`, not as anything mentioning memory --
and only then came `Error #032 -- Memory Full` and a `PACK` failing with
`Error #051`. One procedure per session also matches the documented rule that
a second `PACK` of the same procedure in one session fails.
Combined `tally` after both runs: `CONF6809 totals: PASS=22 FAIL=2 SKIP=0 ERROR=0`.

## The t09 FAIL is real, and it is this system's, not the test's

`t09` is the only test this system does not pass, and it fails the same way
every run: the counter lands short of 200. Seven runs recorded here and in the
sessions around them gave 173, 175, 178, 181, 183, 190, 194 -- never 200,
never the same number twice. That variability is itself the signature of a
genuine race rather than a fixed off-by-something in the test.

Three things separate this from a harness artifact:

- **The run proves its own preconditions.** Both racers signal completion
  through their own separate files, created after their last write, so the
  count is only ever read once both have finished. A racer still running
  reports `ERROR`, not `FAIL` -- a distinction this suite draws deliberately,
  and one that fired for real during development.
- **The contended record is not at end of file.** The fixture is two records
  and the racers work on the first, keeping this on the ordinary record-lock
  path rather than the separate EOF-lock mechanism `t10`/`t11` cover.
- **Neither racer traps errors.** On this BASIC09 an untrapped error drops
  into Debug Mode, which would visibly halt the run. Both racers finish
  cleanly, so the lock is not erroring -- it is simply not serializing them.

This has been independently root-caused in this project outside the suite:
NitrOS-9's record-lock retry path re-presents a clobbered byte count after a
park, degenerating into the "dismiss" request, so a woken waiter proceeds
holding no lock at all. A four-byte register-restore fix takes the loss to
zero across repeated runs. **That fix is not on the disk this suite was run
against, and should not be**: what is recorded here is what a recipient
running stock NitrOS-9 V3.3.0 will see, which is the number worth comparing
against.

`t10`, `t11` and `t12` pass here, so this system honors the EOF lock in the
update-mode case, in the write-only case, and against a second write-only
writer aiming at the locked end -- while failing the general record-lock case.
The two are separate mechanisms in the implementation as well as in the
manual.

## What this run also confirmed, beyond the two above

- **A deliberately broken expectation reports FAIL, not a false PASS.**
  `t01open.a`'s `EXPECT` was changed from 216 to 99, the image rebuilt, and
  run: `RESULT t01 FAIL  obs=00216 exp=00099`. Reverted and reconfirmed PASS
  before shipping.
- **Same, for the two record-locking tests that pass here.** A test that has
  only ever returned PASS has never shown that its comparison can come out the
  other way. `t10eoflk` and `t11wlock` were rebuilt with `xpc = 8` in place of
  7, packed on the guest, staged (byte counts identical, module CRCs
  `$B12C1B`->`$15CBD5` and `$BDED82`->`$DB9DAD`, and the staged module compared
  byte-for-byte against the perturbed build so the image could not be carrying
  a stale copy), and run: `RESULT t10 FAIL  obs=00007 exp=00008` and
  `RESULT t11 FAIL  obs=00007 exp=00008`. `obs` stayed at the genuinely
  measured 7 in both, so the observation is measured rather than assumed.
  Restored byte-exact and reconfirmed PASS before shipping.
- **Same, for `t12`, shown when the test was written rather than on this
  run.** `t12wser` blocks a second write-only writer against the end of a
  file another process holds; the same test with that write moved to offset
  0 -- far ahead of the holder, where the manual gives no reason to block --
  reports `RESULT t12 FAIL  obs=00000`. So the PASS above is a measured
  wait, not a comparison that cannot come out the other way. The extent of
  the lock was measured separately before the test was written, one fresh
  boot per case: a contender is blocked at the holder's own record and at
  the new true end of file, and is not blocked at offset 0 -- the lock
  covers from the holder's write to the end of the file, and is dropped by
  the holder's first not-at-the-end access rather than by its close
  (`test/6809-live-verification/eoflock-extent/`).
- **A missing prerequisite reports SKIP, not FAIL.** Logging in with a
  blank username matches this disk's own wildcard `,,0,...` `SYS/password`
  entry (flat user ID 0, this system's only superuser -- `procs` showed
  User Number 0 for that session). Running `t05fna` directly under that
  login: `RESULT t05 SKIP  operator's user ID is 0 (superuser) -- guard
  bypassed` -- the permission-denial this test checks cannot fire for a
  superuser, so it correctly declines to claim a result rather than
  reporting a misleading FAIL.
- **The image writes nothing outside itself.** `/DD`'s own top-level
  directory listing (`dir /dd`) was captured before and after a full
  prebuilt run, `rebuild`, and the SKIP login above, all in one continuous
  guest session -- byte-for-byte identical both times (prompt text aside).
  Re-checked for the run recorded above by a different route, which is the
  stronger form: the guest disk that run booted is a clone of a golden
  master, so with XRoar stopped both were read host-side with ToolShed and
  their listings compared directly. `/DD`, `/DD/CMDS` and `/DD/SYS` came
  back identical -- same entries, same owners, same dates, same byte counts,
  by both `os9 dir -e` and a plain `os9 dir` name set (1,177 entries across
  the three) -- with exactly one difference, the expected and disclosed one:
  the test harness itself (not the suite) rewrites `SYS/inetd.conf`'s listen
  port on its own private disk clone before boot, to keep concurrent test
  runs from colliding on one TCP port (`tools/nitros9repl.sh`'s
  `sync_chan_port`). That edit lands only on the harness's disposable clone,
  never on a recipient's real disk, and is unrelated to anything the suite
  itself writes -- the suite's own writes stay entirely under `/X0/SCRATCH`
  and `/X0/RESULTS`, i.e. inside the conformance image, never `/DD`. Scope,
  stated rather than implied: this compares those three directories, the
  ones a run could plausibly touch, not every directory on a 128 MB disk.

## A caveat found while producing this file

Two live-driven directory listings of a large NitrOS-9 directory over this
DriveWire channel (`dir -e` on the whole `/DD` root, and plain `dir` on
`/DD/CMDS`, both hundreds of entries) crashed the guest session outright
(the DriveWire server and channel windows both exited; on `/DD/CMDS` the
entire session did). Reproduced twice, from a cold boot, as the very first
command issued. This is a **host/harness-side artifact of the test
transport** (likely the virtual-serial polling falling behind under a
large one-shot output burst), not an OS-9 or CONF6809 behaviour -- neither
the disk nor the suite writes or reads anything that large. Worked around
here by using plain, narrow `dir` calls and a host-side ToolShed read of
the (real, non-DriveWire) `/DD` partition instead -- the same route the
untouched-`/DD` check above now takes. This is documented in the `os9-dev`
skill's `6809/using-nitros9-repl.md` as an output hazard distinct from the
input-burst one.
