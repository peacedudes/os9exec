# Self-hosted 6809 conformance suite — design

**Date:** 2026-07-25
**Status:** design approved, implementation not started

## Goal

Ship a small RBF disk image that anyone with a working OS-9/6809 system can
mount and run, unattended, in three commands — and that reports, per test,
whether the system did what Microware's documentation says it does.

The image is meant to travel: to DrPitre, to anyone with real OS-9 hardware.
Its value is entirely in the answers it brings back, so it is built to
surface disagreement, not to demonstrate success. A run that is green
everywhere has told us almost nothing; a run with three FAILs has told us
where to look.

## What this is not

- **Not a port of the existing corpus.** `test/6809-live-verification/` is an
  investigation kit — 42 of its 51 `.bas` files are `rl-*` record-locking
  scenarios, several of them near-duplicate iterations, with the choreography
  and the judgement living in host-side bash and python that cannot travel.
  It stays exactly as it is; this suite is written fresh alongside it.
- **Not a regression suite for os9exec.** `make test` already does that.
- **Not exploratory.** Every test asserts a *documented* behaviour. A probe
  with no citation does not ship.

## Oracle policy

Microware's published documentation is the standard. os9exec and NitrOS-9 are
reimplementations and are candidates for defect, never authorities — this is
the standing rule in the `os9-dev` skill and it is what makes a FAIL on real
hardware meaningful rather than embarrassing.

Consequences, all of them load-bearing:

- **No test ships without a citation** — document and section — recorded in
  `DOCS/claims`. If the documented behaviour cannot be pinned down, the test
  is dropped from the suite rather than shipped with a guessed expectation.
- **No verbatim manual text travels on the image.** Citations name the
  document and section; the expected behaviour is stated in our own prose.
  The image goes to third parties, and Microware is an active company.
- **Only NitrOS-9 can produce a baseline for these tests.** os9exec is the 68k
  emulator and cannot run 6809 modules at all. Where a claim has *also* been
  tested on 68k under os9exec, that is recorded as a cross-target note, not as
  a second run of the same test — the two targets are different evolutionary
  stages and a 6809 result is not a 68k result.
- A test whose baseline is uncertain still ships. Those are the interesting
  ones.

## Scope: two phases, both committed

Phase 1 is a vertical slice that proves the machinery. Phase 2 adds record
locking, which is not optional — verifying that RBF's automatic locking
behaves as documented is a required outcome of this work, and it is the
single most valuable thing the image can ask of real hardware. It is second
only because it needs multi-process choreography working guest-side, which is
the hardest machinery to trust.

### Phase 1 — the vertical slice

Roughly ten tests, chosen to exercise every part of the machinery before any
scale-up — an assembly module, a BASIC09 procedure, a positive verdict, a
negative verdict, a multi-step file scenario. Shallow on any one subject by
design.

| Area | Mechanism | Claim under test |
|---|---|---|
| Syscall contract | asm module | `I$Seek` position-register convention |
| Syscall contract | asm module | `I$Read` at end of file reports `E$EOF` (211) |
| Syscall contract | asm module | `I$Open` of an absent pathlist reports `E$PNNF` (216) |
| Error path | asm module | Write to a path opened read-only reports `E$BMode` (203) |
| Error path | asm module | Open without adequate permission reports `E$FNA` (214) |
| BASIC09 | packed module | INTEGER range and documented overflow behaviour (16-bit on 6809) |
| BASIC09 | packed module | Documented error code for integer division by zero |
| BASIC09 | packed module | A documented string-function boundary condition |
| RBF | packed module | Seek past end of file, then write — documented extension behaviour |
| RBF | packed module | Read at EOF on a read-only path — the `rl-eof1` question, as a citation-backed conformance test |

Final selection happens at implementation time and is governed by the citation
rule above: a slot whose documentation cannot be pinned is replaced, not
guessed. The mix of mechanisms is the requirement; the individual claims are
not.

### Phase 2 — record locking

RBF locks byte ranges with no call at all, as a side effect of ordinary reads
and writes on a path opened for **update**. That is a documented design
promise, not an implementation detail, and it is exactly the kind of claim
that a reimplementation can appear to honour while diverging under contention.

Three tests, each asserting a documented behaviour:

| Claim under test | Shape |
|---|---|
| **Lost-update races cannot corrupt an update.** Two processes each doing read-modify-write on the same record are serialized by the read's automatic lock and the write's release | Two racers, a fixed number of increments each; the final value must equal the total exactly |
| **The EOF lock.** A write landing at current EOF takes a ghost lock where no data exists yet, so a reader that catches up waits at the edge instead of mistaking "caught up" for "writer finished" — while the real content stays readable throughout | Slow producer, following consumer; the consumer must block at the edge, not read short |
| **Only update-mode opens participate.** Read-only and write-only paths take no lock and wait on nothing — deliberate, so two independent appenders interleave freely rather than excluding each other | Two write-only appenders must both complete without blocking |

Two hard-won constraints carried over from the existing `rl-*` work — the
lessons travel even though the files do not:

- **Each racer must self-report its own completion.** Reading the shared
  counter alone cannot distinguish a lost update from a racer that had not
  finished yet, and at these speeds the shell returns long before a
  backgrounded racer is done. Every short read then looks like data loss. A
  run only counts when every racer has declared itself finished.
- **Every run must prove its own preconditions.** A producer left over from a
  previous run holding the data file open makes the setup step fail, leaves a
  stale file in place, and produces numbers that look exactly like results.
  The test verifies its data file really was recreated by this run before it
  believes any count.

A timing-sensitive test that cannot establish its preconditions reports
`ERROR`, never `FAIL` — a fact about our harness must never be reported as a
fact about his system.

## The test contract

Every test is a standalone program that writes exactly one result line to
standard output and exits:

```
RESULT t03 PASS  obs=00211 exp=00211  I$Read at EOF reports E$EOF
RESULT t04 FAIL  obs=00203 exp=00211  I$Read past EOF on a read-only path
```

Numeric fields are **five digits, zero-padded** — fixed-width columns diff
cleanly, and the emitter's decimal routine produces that form natively. The
line buffer holds 128 bytes, which is the constraint on how long a
description may be; a test whose line would exceed it is a test whose
description needs shortening, not a buffer to be enlarged silently.

- **Verdicts:** `PASS`, `FAIL`, `SKIP` (a prerequisite is absent — the reason
  is stated), `ERROR` (the test itself broke, which is not a finding about the
  system under test).
- **Observed values are always printed**, including on PASS. A disputed
  expectation still yields a usable datapoint, and a FAIL caused by *our*
  wrong expectation is then visibly our fault rather than his machine's.
- Tests may emit additional `NOTE` lines. Nothing else goes to stdout.
- **Every test is standalone**: it creates what it needs in `SCRATCH/` and
  cleans up. No test depends on another having run, so a single test can be
  rerun in isolation with `runone` and a partial run is still valid.

## Image layout

A single raw RBF image, 360K, 256-byte sectors — what DriveWire moves, and
what `mount -k` produces by default.

```
readme          what this is, the commands to run it, what it writes,
                and the fact that it writes nowhere else
runall          procedure file: every test, appending to RESULTS/report
runone          procedure file: one test by ID
rebuild         procedure file: rebuild CMDS/ from SRC/ with the host system's
                own asm and basic09, then rerun
CMDS/           prebuilt modules — the tests, plus the tally program
SRC/            the exact sources CMDS/ was built from
DOCS/claims     per test: ID, document and section cited, expected behaviour
DOCS/ourruns    what NitrOS-9 produced on our harness, labelled candidate,
                not standard
SCRATCH/        the only place tests write working files
RESULTS/report  written by the run
```

Directory names are capitalized and file names are not, per OS-9 convention.

## Prebuilt and source, both live

`CMDS/` runs immediately, with no toolchain required beyond a shell. `SRC/`
carries what built it, and `rebuild` regenerates the modules using the host
system's own `asm` and `basic09` and reruns the suite.

**A difference between the prebuilt run and the rebuilt run is itself a
finding** — it is the cheapest test of his toolchain we get, and it costs one
procedure file.

## Ownership and permissions

Verified live during design, and the answer to the one open question in the
predecessor's recipe (`rbf-image-over-drivewire-verified`, open item 1):
files created after a `login` inside a build procedure are stamped with that
account's ID. `dir -e` shows `1.7` for files written after `login claude` and
`0.0` for one written before.

- Everything on the image is owned by a **non-super** ID — `1.7`, using the
  `claude` account already present in `h0/SYS/password`, so the build never
  writes to the system disk.
- **Public bits are mandatory, not cosmetic.** A file created by `1.7`
  defaults to `------wr`: no public read. On a machine where the operator is
  neither `1.7` nor super, the disk would be entirely unreadable. The build
  sets public read throughout, public execute on `CMDS/`, and public write on
  `SCRATCH/` and `RESULTS/`.
- Attributes are set **on the RBF side**, after the bytes land. A host
  directory has nowhere to store an owner ID and its mode bits do not mean
  what OS-9 attributes mean, so anything set host-side is lost in transit.
  `attr` set/clear is inverted between targets — on 68k, `-e` sets and `-ne`
  clears — and the build runs under os9exec, so it uses the 68k spelling.
- **Nothing in the suite runs privileged.** A test that would need super
  reports `SKIP` with the reason stated.
- The runner records the **running** user in the report. Results are then
  attributable, and if the operator happens to be `1.7`, that is visible
  rather than silently changing which permission bits were exercised.

## Safety properties

These are what make the image safe to hand to someone else, and each is a
testable property of the build, not an intention:

- **Nothing outside the suite disk is ever written.** No test touches `/dd`.
- No non-ASCII bytes anywhere on the image.
- No verbatim Microware text, per the oracle policy above.

## Build pipeline

One make target, `selfhost-6809`, running host-side:

1. **`lwasm`** cross-assembles `SRC/*.a` into OS-9 modules. The guest
   assembler is not an option for the build: `rma` hangs indefinitely under
   our XRoar setup (ROADMAP, reproduced 6+ times), and the lwasm escape hatch
   is already confirmed working on this machine.
2. **BASIC09 sources are packed on our live NitrOS-9** via
   `tools/nitros9repl.sh`. There is no host-side BASIC09; this step needs a
   booted guest, and it is the one part of the build that is not hermetic.
3. **Every text file is converted to CR-only** before it goes on the image.
   This is not optional and it is not cosmetic: a procedure file with LF
   endings is *one line* to OS-9. The shell echoes the entire file and runs
   nothing, with no error at all. Verified during design.
4. **os9exec assembles the image.** It is the 68k emulator, but the RBF
   on-disk dialect is the same one 6809 reads — already demonstrated end to
   end, and a free cross-implementation interop check as a side effect. It is
   driven non-interactively by a CR-only
   procedure file (`os9exec shell /h1/<proc>` — the same mechanism
   `os9repl.sh` uses to boot): `mount -k=360k`, copy the tree in, `login` to
   the non-super account, set attributes, verify with `dir -e`.
5. **Serve it back to our own NitrOS-9** over DriveWire (`--disk0`, via
   `NITROS9REPL_EXTRA_DWCLI`) and run the suite end to end as a non-super
   user.

## Verification protocol

The suite is not believed until each of these has been observed:

1. A full run under NitrOS-9 completes and writes `RESULTS/report`.
2. **One expectation is deliberately broken and the report says `FAIL`.**
   Non-negotiable. Three checks in this codebase turned out to be incapable of
   failing; a green suite that cannot go red is not evidence.
3. **A missing prerequisite produces `SKIP`, not `FAIL`** — likewise proven by
   removing one.
4. The run is performed as a non-super user, and the report records which.
5. `rebuild` is exercised at least once, so we know it works before someone
   else is the first to try it.
6. The image is confirmed to have written nothing outside itself.

## Distribution

The recipient mounts the image on a DriveWire drive, then:

```
chd /x0
chx /x0/CMDS
runall
```

He sends back `RESULTS/report`, or the whole image. The report is plain text,
one line per test, diffable against ours.

## Risks

- **The BASIC09 pack step needs a live guest.** If the XRoar/DriveWire harness
  is unavailable, the build cannot complete. Mitigation: packed modules are
  committed alongside sources, so a rebuild is only needed when a BASIC09 test
  changes.
- **Our expectation may be wrong**, producing a FAIL on a correct machine.
  Mitigated by printing observed values always, and by the citation rule — a
  disputed test can be adjudicated against the document it names.
- **`1.7` may collide** with a real account on the target system, silently
  changing which permission bits apply. Mitigated by recording both the file
  owner and the running user in the report.
- **Phase 2 runs on hardware of unknown speed.** A real machine may be far
  faster or slower than our emulated one, so any test that depends on a racer
  still running when another starts is fragile. Mitigated by self-reported
  completion rather than sleeps, and by the rule that a test unable to
  establish its preconditions reports `ERROR` rather than `FAIL`.
- **The suite may be run write-protected**, in which case `RESULTS/report`
  cannot be written. The runner prints the same lines to the terminal, so the
  run is still recoverable by capture.

## Follow-on, explicitly out of scope here

Scale-up beyond phases 1 and 2, and a **68k sibling image — a separate
session's work**, not this one's. Each is its own spec once the machinery has
been proven by a real run on someone else's hardware.
