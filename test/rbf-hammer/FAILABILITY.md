# Failability ledger

Every check in this harness must be shown going **red** before its green is
counted as evidence. This file records how each one was made to fail.

Why this file exists: four checks in this codebase have been found unable to
fail, and the closest relative of this harness — the `dogfood-lostupdate-*`
counter race — passed **600/600 against code with no record locking at all**.
It was an exercise, not a test, and nothing about its output said so.

**A check with no row here is not coverage.** Do not describe it as passing.

There are two separate questions, and the counter race passed the first while
failing the second:

1. **Can the oracle detect bad data?** Prove with a mutation or a synthetic
   corrupt input.
2. **Can the scenario ever produce bad data?** Prove by running it against a
   deliberately broken emulator. This is the one that matters. An oracle that
   is never handed anything to complain about is green forever.

Injected-defect builds are scratch binaries built to `/tmp`, never committed.
Record the exact mutation so it can be reproduced.

---

## Question 1 — oracle detects bad data

| Check | Mutation applied | Result | Date |
|---|---|---|---|
| `Verifier.verify` (all 7 corruption tests) | `return []` as the first statement of `verify` — the "always clean" failure mode that made the counter race useless | **7 of 11 tests went red.** The 4 that stayed green correctly do not exercise `verify` (`testRecordRoundTrip`, `testEncodedRecordIsFixedWidth`, `testChecksumFitsIn16Bits`) or correctly expect a clean verdict (`testCleanRunHasNoViolations`) | 2026-07-20 |

| `GuestFormatTests` (guest/host byte-format agreement) | `Record.width` 64→63 and `payloadWidth` 44→43 | **All 4 tests went red** against the real guest-produced fixture. First run of this mutation also exposed a genuine defect in the test itself — see below | 2026-07-20 |

| `ScenarioTests` (all 6, end-to-end through os9exec) | Adapter substitutes `@COUNT@` as `worker.count - 1`, so the guest writes one fewer record than the scenario claims | **All 6 went red** (15 assertion failures). Proves the full path is live: the guest really runs, the file really comes back, and the host really compares | 2026-07-20 |

### Harness defect found by adding a multi-worker scenario

The single-worker scenarios passed while the four-worker ones failed, and the
cause was the harness, not RBF: every file was being checked against **all**
workers' expectations, so `w1.dat` was faulted for not containing workers 2-4.

Worth keeping because the fix is asymmetric and the wrong direction is silent:
expectations must be **per file**. Too broad, and separate-file rosters fault
spuriously (loud, obvious). Too narrow, and a roster sharing one file would
excuse a genuine foreign record as a phantom writer (silent, and exactly the
kind of miss this harness exists to prevent). `Scenario.expectations(forFile:)`
now carries that reasoning.

### Defect found BY the mutation, in the test code

The first run of the width mutation **crashed with signal 5** instead of
failing. `testGuestChecksumsMatchHostChecksums` computed frame bounds with
`data.index(startIndex, offsetBy: start + width)` and walked past the end when
the stride no longer divided the data evenly.

That is not a mutation artifact — it is how the test would have behaved on a
**genuinely torn file**, which is one of the exact conditions this harness
exists to detect. The test would have crashed the run rather than reporting the
tear. Fixed by giving `Record` a single bounds-safe `frames(in:)` helper that
both the verifier and the tests use, so a short final frame is produced
deliberately rather than indexed into existence. Re-run: 4 clean failures, no
crash.

### Negative result, recorded deliberately

| Claim | Mutation | Outcome |
|---|---|---|
| "Strict-ASCII decode in `Verifier` is required to catch garbage bytes" | Replaced `String(bytes:encoding:.ascii)` with lenient `String(decoding:as:UTF8.self)` | **Test still passed — the claim is not supported.** A replacement character corrupts the sequence field, the field parse fails, and `.tornRecord` is reported by a different route. Strict ASCII is retained on precision grounds (fails at the corruption, cannot merge several bad bytes into one replacement char) but **no test currently distinguishes the two implementations**, and `testGarbageByteIsReportedAsTornRecord` must not be cited as if it did. |

This row is the point of the ledger. The test looked like it guarded the
strict-ASCII choice, and it does not. Without the mutation check it would have
been quietly miscredited.

---

## Volume findings — and a false accusation caught

**The 3-second run was a smoke test, not a hammer.** Defaults were 8 records per
worker. Raising the volume immediately produced failures:

| Records/worker | Result |
|---|---|
| 8 | 11/11 pass, ~3s |
| 50 | 3 scenarios failed |
| 200 | 1 scenario failed |
| 500 | 17 assertion failures across the set |

### The N=50 failures were MY BUG, and they looked exactly like an RBF defect

Presentation: one worker's entire 50-record range read back as zeros, while that
worker reported success. Intermittent. Only on shared files. Only on RBF images.
That is a textbook "RBF loses data under concurrent writers" report.

It was none of those things. Running the worker ALONE failed too, with
`E$EOF (211)`:

**OS-9 cannot write into a hole.** `SEEK`ing past the current end of file and
writing fails; it does not extend sparsely. A `slot` worker whose range starts
beyond the current end dies immediately. Workers 1 and 2 survived only because
their ranges begin at or before the existing end, which is why it looked
worker-specific and intermittent rather than systematic.

The skill documents this ("EOF errors from `SEEK`ing past the current end of a
sparse file") and I did not read it before designing the slot layout.

Fixed by making the provisioning worker PRE-EXTEND the file to its full size.
The filler is deliberately not a valid record, so a slot that never gets written
still reads back as a torn record and is DETECTED rather than excused.
After the fix: 5/5 clean runs of all 11 scenarios at N=50.

**Lesson for this harness: a green oracle proves nothing, and so does a red one
until the harness itself is exonerated.** This would have been filed as an RBF
data-loss bug against freshly merged code.

### Open, NOT yet classified

The N=200 and N=500 failures are **unclassified**. `testTwelveWorkersShareOneFile`
passes at N=500 in isolation, so the failures are either in other scenarios or
emerge only when the set runs together. Candidates: the 500K image is tight for
12x500x64 = 384000 bytes, and disk-full is a real condition worth testing
deliberately rather than tripping over accidentally. Do not describe these as
RBF defects until each one has been isolated the way the N=50 case was.

## 6809 / NitrOS-9 (Adapter6809) — 2026-07-20

Real NitrOS-9 on an emulated CoCo3. Every run works on a copy-on-write clone of
the golden master; the master is never booted by the harness.

### What IS established

| Check | Mutation applied | Result | Date |
|---|---|---|---|
| `testSingleWorkerAppendsOn6809` (whole 6809 path end to end) | `@COUNT@` substituted as `worker.count - 1`, so the guest writes one fewer record | **RED, precisely**: `worker 1: wrote 8, found 7` + `missing sequences [7]`. Proves the guest really runs, the file really comes back out of the `.ide` container, and the host really compares | 2026-07-20 |

`testFourWorkersShareOneFileOn6809` passes (11.5s) and shares the mutation
above, since both go through the same render/inject/collect path.

### The record lock IS measured on 6809 — MEASURED + PROVEN FAILABLE 2026-07-20

**`rmw`/`rmwfree` now run, and the control loses updates.** The earlier reading
that `.nilWrites` "never interleaves" was measured at **nap 1** — a single `/nil`
write, which opens no scheduling window at all. A *large* burst does interleave,
cheaply, with no process fork. The operating point was found by bisection:

| napMode | nap | Roster (4×10) | Unlocked control tally | Verdict |
|---|---|---|---|---|
| `.nilWrites` | **1** | 11s | 40/40 — perfect | window is one write; never interleaves. This is the misread the old note generalised from |
| `.nilWrites` | 100 | 10s | 20/20 (at 4×5) — perfect | window still too narrow |
| `.nilWrites` | **400** | ~10s | **26–28/40 — loses ~13 every run** | interleaves reliably AND fast. **The operating point.** |
| `.nilWrites` | 1000 | control 11s; **locked 606s TIMEOUT** | control loses (11/20 at 4×5) | window so wide the *locked* roster starves — worker 2 never ran, tally 15/20 was a timed-out run, NOT a lost update. Do not cite it as one |
| `.sleep` | 1 | 4×25→75/100 in 300s; 4×10 >10min | loses | genuine window but forks a `sleep` per increment at ~0.26/s — too slow. `PACK`+`runb` was the proposed fix and is NOT needed: `.nilWrites` nap 400 gets there without it |

At nap 400, both directions are demonstrated and both are fast (~10s):

| Configuration | Final tally (4×10) | Runs |
|---|---|---|
| `rmwfree` — no lock taken | **26–28** (loses ~13 of 40) | 6+, every one lost updates |
| `rmw` — update-mode auto-lock | **40** — perfect | 5+, every one perfect |

`testUnlockedReadModifyWriteDoesLoseUpdatesOn6809` is the permanent guard: if it
ever keeps a full 40, the `.nilWrites` window has stopped opening and every other
6809 lock result must be treated as meaningless again — exactly the 600/600
counter-race trap. Why nap 400 interleaves where nap 1 does not: the burst has to
span several of the emulated 100Hz preemption ticks for a competing worker to be
scheduled into the window between the read and the write. One write spans none.

**Open robustness caveat, recorded honestly — the control is load-sensitive.**
Twice, both while other `swift test`/build processes were loading the host
(1-min load ~8), the control run stretched to ~152s (≈15× normal) and reported a
**perfect** 40/40 — zero interleave — which FAILS its assertion. At normal load
(<~2) the pair is solid: 3/3 back-to-back pair runs and a full-suite run all
passed at ~10s each, control losing ~13 every time. **The failure direction is
SAFE**: it faults toward a loud false alarm (the control refusing to certify),
never toward silently passing a broken lock. The *mechanism* is NOT established —
a uniform emulator slowdown should not change the *relative* timing that drives
the interleave, yet under load it did. Per this ledger's own discipline that is
left as an observed, unexplained caveat rather than given a fabricated cause.
**Do not run the 6809 lock pair as a gate on a loaded host without re-checking;
prefer a quiet host, or treat a single 40/40 as inconclusive-under-load rather
than as a lock pass.**

### Two harness defects that looked exactly like RBF data loss

Both produced "concurrent writers lose everything" and both were the harness.

**1. A `What?` flood that read as total data loss — cause NOT the one first
recorded.** Driving the roster with `shell #32k </DD/run.s` made exactly ONE
racer of four run — a *different* one each run — while the rest never started and
the shell emitted a burst of `What?`. The file came back correctly pre-extended
and entirely unwritten, which reads precisely as RBF dropping every concurrent
write. Fixed by typing commands at the shell one at a time and waiting for each.

The first cause recorded here — "a backgrounded child inherits the parent's
stdin and file position, so the children eat the procedure file" — was **tested
and is FALSE**, and is kept only as a caution. Four isolated procedure files
(single/double background `echo`; single/double background `basic09 <file&`,
the exact roster shape) ALL launched their background jobs and completed
cleanly. A procedure file backgrounds fine. The real trigger of the flood was
never pinned down; the leading candidate is channel corruption from sending
`key` faster than the guest drains it (a documented gotcha of this REPL), which
the one-at-a-time-with-waits driving happens to also cure. So the fix is a
REPL-pacing workaround, not an OS-9 shell fact.

This row is itself a ledger lesson: a plausible harness explanation was written
into three files (this one, the adapter comment, the skill) without being made
to fail on demand. The owner asked "did you try `&`?", a five-minute live test
disproved it, and the retraction is recorded rather than quietly deleted.
**Successor task: find what actually caused the flood, make it reproduce, then
replace this caveat with the real cause.**

**2. The golden master was booted instead of the clone.** An XRoar survived a
`stop`, a later boot reused it, and a run wrote to the golden master while every
host-side check still looked right — the injected scripts were in the clone, so
the guest simply reported "path not found". One stray file reached the master and
was deleted through RBF itself; free-sector count verified unchanged afterwards.

`assertAttachedImage` now refuses to run unless `lsof` confirms XRoar holds
*this run's* clone. It has been seen firing. Note the first version of that guard
was itself wrong — it compared `/var/...` against lsof's `/private/var/...` and
rejected the very clone it had just made — so **both sides are canonicalised**.

### Environment facts verified live (all previously unconfirmed)

- **`SHELL` DOES exist in 6809 BASIC09** and really forks — `SHELL "echo ..."`
  produced its output. The handoff listed this as unconfirmed; `.nilWrites` is
  not needed as a fallback for availability (only for speed, and it is too weak
  to interleave).
- **ToolShed cannot read the `.ide` container.** The OS-9 partition starts at
  byte 323,584 (verified: RBF LSN0, volume "NitrOS-9 EOU 6809", byte-identical
  to the separately extracted `partition.img`). `dd` out, edit, `dd` back —
  about 1.8s round trip.
- **`cp -c` clones the 134MB image in ~5ms** on APFS, so a per-run throwaway
  disk is free.
- `Rammer`/`R0` (RAM disk) and `Nil` exist in the boot module directory, but
  provisioning the RAM disk is unverified, so `Backend6809.device` REFUSES
  `.ramDisk` and `.hostDirectory` rather than silently using `/DD`.

## Question 2 — scenario can produce bad data

### What the `slot` scenarios do NOT test — read this before citing them

The eleven `ScenarioTests` all pass, and none of them can detect a missing
**record lock**. Stated plainly so nobody cites them as lock coverage:

`slot` workers write **disjoint** byte ranges (worker *i* owns records
`[(i-1)*count, i*count)`). `LockHolder` in `file_rbf.c` only reports a conflict
when two paths' locked extents **overlap**. Disjoint ranges never overlap, so
removing record locking altogether would leave every one of these scenarios
green.

That is not a defect in the scenarios — it is a limit on what they cover:

| These scenarios DO exercise | They do NOT exercise |
|---|---|
| the FD-sharing ring across many paths on one file | record lock acquisition |
| shared sector buffers and their invalidation | lock conflict / sleep / wake |
| segment list and published size under concurrent extends | `E_DEADLK` refusal |
| the device allocator with many concurrent writers | the EOF wait |

The disjointness is deliberate and still correct: BASIC09 has no seek-to-end, so
racing appenders would all write at offset 0 and overwrite each other, which
would prove the test wrong rather than the locking right.

**A genuine lock test needs OVERLAPPING read-modify-write on the same records.**

### `rmw` scenario — PROVEN FAILABLE 2026-07-20. Record-lock coverage EXISTS.

N workers each read-modify-write the SAME record, with the nap held BETWEEN the
read and the write so the lock stays open across a real scheduling window.

**The control needed no emulator rebuild.** RBF auto-locks only on UPDATE-mode
paths, so doing the read through a READ path and the write through a separate
WRITE path is an unlocked read-modify-write — `rmwfree` — with the emulator
completely untouched.

| Configuration | Final tally | Expected |
|---|---|---|
| `rmwfree` — no lock taken | **20** | 100 (80 updates lost) |
| `rmw` — update-mode auto-lock, tick ON | **100** | 100 |
| `rmw` — update-mode auto-lock, tick OFF (`-q`) | **100** | 100 |

Both directions are demonstrated: the race genuinely interleaves and destroys
data when unlocked, and the automatic record lock genuinely prevents it, with
the tick on AND off. `testUnlockedReadModifyWriteDoesLoseUpdates` is a
permanent guard — if it ever starts passing a full tally, the race has stopped
interleaving and every other lock result must be treated as meaningless again.

**Why the old counter race got 600/600 and this does not:** it had no yield
between the read and the write, so the read-modify-write completed within one
scheduling quantum every time. The nap is the whole difference.



Nothing here yet. Scenarios arrive in Tasks 3–4 of
`docs/superpowers/plans/2026-07-20-rbf-lock-hammer.md`; each one needs a row
before it counts.

Planned injections, one per defect class:

| Injection | Scenario class it must break |
|---|---|
| record lock acquisition removed | `update` lost-update scenarios |
| EOF wait replaced by immediate `E$EOF` | `follow` scenarios |
| ring invalidation on dirty sector skipped | multi-path visibility scenarios |
| cluster free-list update skipped on delete | allocation round-trip |

**A scenario that stays green under its matching injection is broken.** Fix the
scenario. Do not rationalise it as "the injection was not severe enough"
without first proving the workers actually contended — "ran clean" and "never
overlapped" are indistinguishable from the outside, which is exactly how the
counter race stayed green for 600 runs.
