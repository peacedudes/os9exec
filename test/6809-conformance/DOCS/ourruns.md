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
- Emulator: XRoar 1.11, headless, full speed (`-no-ratelimit`).
- Bridge: DriveWire, `drivewire-cli` (`drpitre/drivewire`, stock upstream
  `main`), tunneled over XRoar's becker port. The conformance image itself
  is served to the guest as a second virtual disk (`--disk0`, appearing as
  `/X0`) alongside the guest's own real boot disk (`/DD`) -- see
  `tools/selfhost6809/run-on-nitros9.sh`.
- Login: `USER1`, this disk's ordinary non-super account (flat ID 1; ID 0
  is the only superuser on this system -- see the owner-ID paragraph in
  `text/readme`).

## Prebuilt run (`CMDS/`, shipped as-is)

```
RUN prebuilt

         User                     Mem Stack
Id  PId Number  Pty Age Sts Signl Siz  Ptr   Primary Module
--- --- ------- --- --- --- ----- --- ----- ----------------
  3   9     1   128 131 $80    0   31 $31E2 Shell
  4   3     1   128 128 $80    0   31 $1EF3 Procs
  9   7     1   128 129 $80    0   31 $2EE2 Shell
RESULT t01 PASS  obs=00216 exp=00216  I$Open of an absent pathlist reports E$PNNF
RESULT t02 PASS  obs=32768 exp=32768  INTEGER overflow on 6809 wraps modulo 65536 (32767+1=-32768)
RESULT t03 PASS  obs=00211 exp=00211  I$Read at end of file reports E$EOF
RESULT t04 PASS  obs=00203 exp=00203  writing to a path opened read-only reports E$BMode
RESULT t05 PASS  obs=00214 exp=00214  I$Open without permission reports E$FNA
RESULT t06 PASS  obs=00077 exp=00077  I$Seek honors the X:U 32-bit position convention
RESULT t07 PASS  obs=00045 exp=00045  INTEGER divide by zero traps into BASIC09's own error 45
RESULT t08 PASS  obs=00054 exp=00054  a write past a seek beyond EOF extends the file
```

`tally`: `CONF6809 totals: PASS=8 FAIL=0 SKIP=0 ERROR=0`

## Rebuilt run (`rebuild`: SRC/ reassembled with this system's own `asm`,
repacked with its own `basic09`, run from `REBUILT/`)

```
RUN rebuilt

         User                     Mem Stack
Id  PId Number  Pty Age Sts Signl Siz  Ptr   Primary Module
--- --- ------- --- --- --- ----- --- ----- ----------------
  3   9     1   128 128 $80    0   31 $31E2 Shell
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
```

`rebuild`'s own reassembly step: 5 `asm` invocations (the assembly tests),
0 errors each (a handful of harmless warnings, the same count every time);
4 `basic09` `PACK`s (the BASIC09 tests plus `tally`), all successful.
Combined `tally` after both runs: `CONF6809 totals: PASS=16 FAIL=0 SKIP=0 ERROR=0`.

## What this run also confirmed, beyond the two above

- **A deliberately broken expectation reports FAIL, not a false PASS.**
  `t01open.a`'s `EXPECT` was changed from 216 to 99, the image rebuilt, and
  run: `RESULT t01 FAIL  obs=00216 exp=00099`. Reverted and reconfirmed PASS
  before shipping.
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
  Confirmed further, host-side, against the same session's disk image with
  ToolShed: `/DD/CMDS` and `/DD/SYS` carry no entries dated the day of this
  run except one expected, disclosed exception -- the test harness itself
  (not the suite) rewrites `SYS/inetd.conf`'s listen port on its own
  private disk clone before boot, to keep concurrent test runs from
  colliding on one TCP port (`tools/nitros9repl.sh`'s `sync_chan_port`).
  That edit lands only on the harness's disposable clone, never on a
  recipient's real disk, and is unrelated to anything the suite itself
  writes -- the suite's own writes stay entirely under `/X0/SCRATCH` and
  `/X0/RESULTS`, i.e. inside the conformance image, never `/DD`.

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
the (real, non-DriveWire) `/DD` partition instead. Not yet in the skill's
`6809/using-nitros9-repl.md`, which documents the input-burst version of
this hazard but not this output-volume one; worth adding there in a future
session (out of this task's write scope).
