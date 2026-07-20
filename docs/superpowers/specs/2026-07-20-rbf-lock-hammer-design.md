# RBF lock & integrity hammer — design

Date: 2026-07-20
Status: approved (design), not yet implemented

## Why

os9exec's RBF record/EOF locking was rewritten and merged
(`a17cf67..37cbfbf`): paths on one file now share FD state through a ring,
a reader at end-of-file parks and is woken by its writer, a read in update
mode locks the record until its write, and `SS_Lock`/`SS_Ticks` do real
work where they used to be `pNop`. The pre-emptive system tick landed
separately and is now the default.

That is a large change to the most damage-prone subsystem in the project,
and the evidence under it is thin: hand-driven transcripts plus one
deterministic test. Worse, the test people would naturally reach for --
the `dogfood-lostupdate-*` counter race -- **cannot fail**. It passed
600/600 against code with no locking whatsoever, because a cooperatively
scheduled emulator rarely interleaves a read-modify-write. A green result
from it means nothing.

The exposure is not limited to the lock code. The FD-sharing ring touches
sector buffering, segment lists, file size, and attribute propagation --
so anything RBF does may have been disturbed. And on the 6809 side, RBF is
a community-maintained disk manager: a defect there can destroy a real
user's disk. That asymmetry is why this is worth over-testing rather than
spot-checking.

## Goals

- A deterministic regression gate, under one minute, that runs every time.
- A soak harness with iteration count chosen at the call site, for finding
  what is broken now rather than guarding what is fixed.
- The same scenario corpus on both targets: os9exec/68k and NitrOS-9/6809.
- Coverage of all storage backends, since behaviour plausibly differs
  between them: RAM disk, RBF image file, host directory.
- Coverage of both scheduling modes on 68k: tick on (current default) and
  tick off (`-q`, the old default and the documented fallback).
- Never write a real system disk. Never let two concurrent runs interact.

## Non-goals

- Cross-target divergence is **reported, not failed**. Neither
  implementation is an oracle for the other; a difference is a question
  for the owner, not a red test.
- No fix work in this project. Finding and characterising is the
  deliverable; fixes are separate, reviewed changes.

## Architecture

Three layers, so the expensive part is written once and both platforms
share it.

### Layer 1 — guest-side programs (portable, the only per-target risk)

A small number of *parameterized* programs, not one program per scenario.
Scenario variety comes from arguments, not source files.

**`hammer` worker.** Arguments: target file, role, iteration count, sleep
pattern, worker id. Roles:

| Role | What it does |
|---|---|
| `append` | extends the file with self-describing records |
| `update` | opens update mode, read-modify-writes existing records |
| `follow` | reads to end of file and keeps reading (exercises the EOF wait) |
| `hold` | takes an `SS_Lock` and sleeps, to force contention |
| `churn` | creates and deletes files in the same directory |
| `truncate` | shortens the file under other workers |
| `remove` | deletes the file while others hold it open |

**`verify` program.** Reads a file produced by a run and checks the
content invariants below.

Both stay in BASIC09, the dialect known to run on both targets. C is added
only where BASIC09 cannot reach: exact-offset unbuffered `I$Read`/`I$Write`
and direct `SS_Lock` getstat calls. Whether the 6809 C toolchain is usable
for this must be **verified live**, not assumed; if it is not, the C-only
scenarios are tagged 68k-only rather than dropped.

### Layer 2 — scenario table (shared, declarative)

Each entry is data: backend, worker roster (N workers and their
arguments), tick mode, iteration count, applicable targets, expected
invariants. The same table drives both platforms. Adding a scenario is
adding a row.

Worker counts are not limited to two. Scenarios range from a single
process up to rosters large enough to genuinely contend.

### Layer 3 — transport adapters (two, thin)

- **68k**: drives os9exec through the existing Swift harness's `os9()`.
- **6809**: drives XRoar through `tools/nitros9repl.sh`.

Only these two files contain platform knowledge.

## Isolation

Concurrent runs of the gate and the soak must both work. Therefore:

- **68k**: a private device root per run, keyed by process id, extending
  the existing `scratchDisk` pattern in `test/Sources/OS9Tests/main.swift`.
- **6809**: a **golden master** boot/data image is kept untouched. Each
  run clones it, works on the clone, and discards the clone afterward.
  The boot disk is never a test target.

This repo has already been damaged by the absence of this: a shared `h0`
let two suites delete each other's fixtures, producing failures
indistinguishable from real regressions.

## Oracles

All four run on every scenario. A scenario passes only if all four pass.

1. **Content.** Every written record is self-describing: worker id,
   per-worker sequence number, and a checksum over the payload. The
   verifier proves no torn records, no lost updates, no duplicates, no
   sequence gaps within a worker, and no records attributed to a worker
   that never ran.
2. **Structural.** `dcheck` and `free` clean on the device after the run.
3. **Allocation round-trip.** Delete everything the run created, then
   compare the free-space map against the golden master. It must return
   to baseline. This detects leaked clusters, double allocation, and
   orphaned file descriptors -- a class of corruption that is perfectly
   self-consistent and therefore invisible to both oracles above.
4. **Liveness.** No worker hangs. A permanently parked reader is a real
   bug in the new wait/wake path; without this oracle it presents as an
   indistinguishable timeout.

## Making races actually happen

With the tick on (100Hz, the default) pre-emption supplies interleaving.
With `-q` the tick is off and a task essentially runs to completion, so
the corpus must create its own interleaving: workers take **voluntary
sleeps** at points chosen to open the windows that matter -- between a
read and its write in update mode, between an extend and its size
publish, and around close. Sleep patterns are a worker argument, so the
same roster runs both ways.

This matters beyond 68k: NitrOS-9 has no `-q` equivalent and is always
pre-emptive, so voluntary sleeps are the portable mechanism and the
tick-off axis is 68k-only.

## Destructive testing

Explicitly in scope, because allocation-map defects live there: killing a
writer mid-write, exhausting free space, deleting a file while several
processes hold it open, truncating under a reader. Safe by construction --
everything runs against a per-run scratch device or a discarded clone.

## Non-lock RBF stress

Included deliberately, since the FD-sharing ring plausibly disturbed all
of it: concurrent create/delete churn in one directory, files grown past a
segment boundary while being read, free-space exhaustion, delete-while-open,
truncate-under-reader, and directory operations racing file operations.

## Failure behaviour

Default is **stop at the first failure and preserve the disk image**, which
is the shortest path to a reproducer. A `--keep-going` flag runs to
completion and reports a failure rate instead, for answering "is this
one-in-ten or one-in-ten-thousand".

## Runtime budget

Iteration count is a call-site parameter (`N=`). During development it is
tuned fast -- seconds per scenario -- because the corpus will be iterated on
several times and a real defect may surface in three seconds anyway. Once
the corpus is believed correct, the documented soak default is tuned to
roughly ten minutes per platform. The gate stays under one minute.

## Verification that the tests can fail

Non-negotiable, and the reason this document exists. Three checks in this
codebase have been found unable to fail, and the lost-update race is a
fourth. **Every oracle must be demonstrated failing at least once** before
its green result is trusted -- against a deliberately broken binary, a
corrupted image, or an injected fault. An oracle that has never gone red is
not evidence.
