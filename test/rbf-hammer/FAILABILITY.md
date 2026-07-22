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

| `StructuralOracle` (the on-disk damage oracle for the destructive scenarios) | **Real corruption, not a code mutation:** a fresh `mount -k=500K` image was given a file, then a bitmap byte was flipped host-side (`byte 268 |= 0x10`) to mark an unused sector allocated, and `dcheck` re-run | **`dcheck` DETECTED it** (`Sector 000063 ... not in file structure` / `1 cluster not in file structure`) and the oracle reports `.structuralDamage`. Crucially it also caught the oracle being WRONG first: `dcheck` prints `file structure is intact` **alongside** the fault, so a naive "require intact" check passed the corrupt disk. Fixed to also require every allocation-fault phrase absent; the unit test now uses this exact real output | 2026-07-21 |

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

### N=200 / N=500 — CLASSIFIED 2026-07-21: `Error #237 RAM Full`, not disk

The old guess ("the 500K image is tight, disk-full") is **wrong** and is
disproven by the run itself. The worker `ON ERROR` handler added this session
turned the opaque N=500 failure into a self-report: the transcript is now ~900
lines of `hammer: hnap ERROR 237` — **RAM Full**, raised inside `hnap` at its
`SHELL "sleep"` fork. It is process memory, not disk: in the same run the
provisioner pre-extends `shared.dat` to **2000 slots** and all four workers
report `wrote 500`, so allocation and the writes themselves succeed. The cause
is the nap: `.sleep` forks a `SHELL` per increment, and 4 workers × 500
increments is ~2000 forks in flight, which exhausts the 68k guest's free memory
the same way the 6809 procedure-file roster exhausted the CoCo3's.

N=200 passes; N=500 fails — the fork pressure crosses the memory ceiling
somewhere between. It is the SAME defect class as the 6809 flood (an uncaught
`#237` from a `SHELL` fork), and it is **not** an RBF fault: the data written is
correct. Two clean fixes, neither yet applied: give the guest more memory, or
use `.nilWrites` (no fork) for the nap at high N. The writes succeeding under
900 nap failures is itself the `ON ERROR` handler proving its worth — before it,
this was an unreadable flood.

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

**Re-blessed after the PACK+runb change (2026-07-21):** with launches now via
`runb`, the control still loses every time (5/5) and the locked side still keeps
a perfect 40 (2/2), both at load ~5. PACK+runb changed only HOW the worker is
launched, not the nap timing, so the interleave is unaffected — confirmed, not
assumed. Both halves of the gate hold.

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
**This load-sensitivity was root-caused and FIXED — see the editor-build-race
section immediately below.** The mechanism was not an emulator slowdown; it was a
worker stalling in the BASIC09 editor at launch, and PACK+runb removes the editor
from the launch path. The suite now passes at load ~5. The caveat is kept as the
record of how the flake was chased down.

### The load-sensitivity above IS an editor-build race (2026-07-21)

The "unexplained" load caveat now has a mechanism, caught live. Every racer is
built by driving the BASIC09 **editor** from its script file (`e hnap` / `e
hwork` / insert lines / `q` / `run`) on each launch. The racers launch
concurrently, so several editors parse at once — and under host load two of them
race until one spins forever at its `E:` prompt (`E:*` without end), the
editor-phase cousin of the `D:` debugger flood. Caught with an onset snapshot:
workers 3-4 finished, workers 1-2 stuck in a 669-line `E:*` spin, no data lost,
the roster simply hung to the timeout. This also explains the 152s "zero
interleave" control runs above: a worker that stalls in `E:` starts so late the
others have finished, so nothing overlaps.

Confirmed both directions of the trade before fixing it: **serialising** the
launches (a per-worker "up" marker before the next) removed the hang but then the
rmw control kept a perfect tally, because staggered starts no longer overlap and
the lock race never happens. So parse-at-launch cannot be both concurrent (needed
for the race) and unraced (needed to not hang).

**FIXED 2026-07-21 by PACK+runb.** The worker is no longer parsed at launch. Each
racer is built ONCE, sequentially, in a setup phase (`WorkerScript.renderPack` →
`basic09 </DD/w<id>p.s` builds `hnap<id>`+`hwork<id>` and `PACK`s them to a module
`hwork<id>`), and then launched with `runb hwork<id>&` — instant, no editor. The
per-racer id-suffixed names mean no two modules share a name, so concurrent
`runb`s never contaminate each other's resident modules. The setup is sequential
so nothing races the editor; the launches are concurrent so the lock race is
real. Result: the whole 6809 suite is green **at load ~5** (7 tests, 0 failures,
50s), and four-worker runs are 5/5 at ~12.7s — the intermittent hang is gone in
both directions. A second latent hang surfaced once the editor race was cured:
the completion wait was a `w`-per-racer plus one `echo HAMMER-DONE`, and a dropped
key-send of that trailing sequence hung the roster after all the work was already
done. Replaced by waiting on each worker's own `hammer: worker N ...` line
(`waitForWorkers`) — no extra command to drop, and a missing worker names itself.
`renderPack` is 6809-only; the shared template and the 68k path are untouched.

### The lock PAIR keeps a residual load-sensitivity — now guarded (2026-07-21)

PACK+runb cured the editor-build hang, but a full-suite run at 1-min load ~4
showed the rmw **control** (`testUnlockedReadModifyWriteDoesLoseUpdatesOn6809`,
`.nilWrites` nap 400) is still load-fragile for a *different* reason: with no
editor left to race, a racer can still be starved of CPU inside the `.nilWrites`
burst under host load, so the four workers fail to overlap and the roster hangs
to the 600s timeout (the LOCKED half passed the same run in 13.7s — the lock
itself is fine; only the control's interleave is timing-dependent). So the
earlier "green at load ~5" reading held for the editor-race fix but not for the
pair's interleave window. This is the load caveat the P1 notes flagged as
unexplained-but-direction-safe, now pinned to worker starvation rather than
editor parsing.

**Guarded, not papered over:** both lock-pair tests now call
`skipLockPairWhenHostBusy()`, which reads the 1-min load (`getloadavg`) and
`XCTSkip`s above `maxLoadForLockPair` (2.5 — measured rock-solid below ~2,
fragile by ~4-5). Seen fail first: the un-guarded run hung 606s and failed at
load 3.7; guarded, the same two tests skip in 0.001s with the load in the
message. At a quiet host they run and assert as before — the control must still
lose updates or the locked result means nothing. The skip is honest: under load
there is simply no lock verdict, neither a false pass nor a ten-minute hang. The
other eight 6809 tests (destructive, four-worker, single) are NOT timing-gated
and stay green under load — verified live at load ~4 (all passed, 11-14s each).

### ★★ REPRODUCED DATA-LOSS BUG — binary GET/PUT loses updates (2026-07-21)

The hammer now REPRODUCES the live 6809 lost update — the bug the text `rmw`
role was structurally blind to. `testBinaryReadModifyWriteLosesUpdatesOn6809`:
four `rmwbin` workers each run 200 back-to-back `SEEK0/GET/+1/SEEK0/PUT` on ONE
10-byte integer record (binary `GET`/`PUT`, matching `rl-race3`), launched on one
shell line so their first GETs coincide. **Measured 302 and 258 of 800 lost**,
every concurrent run. Pinned with `XCTExpectFailure` (retries up to 3x, bug taken
as present the instant any run loses) so the suite CATALOGS a reproduced
data-loss bug instead of a green pass, and flips loud if RBF is fixed here.

**Why the text rmw was blind (three green-seeking choices, each proven to
suppress it):** a big nap that HELD the lock (serializes cleanly); only 40
updates (loss variance easily hits 0); and `READ`/`PRINT` text I/O, whose lock
path serializes CORRECTLY. Decisive proof it is I/O-verb-specific: at 6 workers
the UNLOCKED text control lost **977/1200** (the workers overlap hard) while the
LOCKED text rmw kept **1200/1200** (the text lock works). Same 4 workers, same
nap=0, same file: text keeps 800/800, binary loses ~250-300. The ONLY variable
is `READ`/`PRINT` vs `GET`/`PUT` — the bug lives in the binary `I$Read`/`I$Write`
lock-retry-after-park path, where a woken waiter re-presents a clobbered byte
count and runs unlocked (root cause: `docs/nitros9-rbf-lostupdate-*`).

**Launch simultaneity is load-bearing in THIS harness (measured, not assumed):**
the one-line concurrent launch loses ~250-300; a per-key-send launch (racers land
seconds apart, because `runb` startup is slow) keeps a clean **800/800 every
time** — the lead worker loads and marks sector 0 before the next racer's first
GET, exactly the stagger that suppresses the race. A native-module reproduction
with ~ms startup does not need this; this one does, so the adapter launches the
racers on a single shell line. (This is why the hammer never saw the bug before:
it was launching the racers the un-buggy way, AND on the text path.)

### RAM: the CoCo3 is now given 2MB, not the stock 512K

`Backend`/`replEnvironment` boots XRoar with `-ram 2048`. A 512K machine leaves
only ~304K free after the resident OS (read straight off `mfree`); a roster of
several `#32k` BASIC09 workers each forking `SHELL "sleep"` exhausts it and the
fork fails `Error #237 (RAM Full)`, which — with no `ON ERROR` — becomes the
debugger flood. At 2MB `mfree` reports **1824K free**, and the procedure-file
roster that always flooded at 512K completes cleanly (9.6s). NitrOS-9 EOU detects
and uses the expansion (the 1520K block at `$80000–$1FBFFF` only exists above
512K). Separately, every worker role now carries an `ON ERROR` that reports the
code to `#2` and exits, so **any** uncaught error — a real RBF error included —
surfaces as `worker N ERROR NNN` instead of an opaque hang-and-flood.

### Two harness defects that looked exactly like RBF data loss

Both produced "concurrent writers lose everything" and both were the harness.

**1. A `What?` flood that read as total data loss — CAUSE NOW CONFIRMED
(reproduced on demand 2026-07-21).** Driving the roster with
`shell #32k </DD/run.s` floods the channel with `What?` and leaves the shared
file pre-extended but entirely unwritten — which reads precisely as RBF dropping
every concurrent write, and is nothing of the kind.

The cause was pinned live, in order, by driving the guest interactively and by an
onset snapshotter that caught the trigger line before the flood buried it
(`RBF_DRIVE=procfile` reproduces the whole thing):

1. **`What?` is BASIC09's interactive DEBUGGER, not the shell.** An uncaught
   BASIC09 runtime error `BREAK`s into the debugger (`D:` prompt). Proven with a
   minimal `x=1/0` program: run with stdin = terminal it drops to `D:` and
   **blocks** (a single `D:`, responds to `q`); run with stdin = a redirected
   script file (`basic09 <file`) the debugger reads past the file's EOF and
   **spins**, emitting `What?` on every empty read — the unbounded flood.
2. **The uncaught error is `Error #237 — RAM Full`, in `PROCEDURE hnap`**, at its
   `SHELL "sleep"` line. The snapshotter caught it exactly:
   `*003E SHELL "sleep "+STR$(ticks)` / `Error #237` / `BREAK: PROCEDURE hnap`.
   `SHELL` forks, and the fork runs the CoCo3 out of memory.
3. **Why procedure-file driving and not one-at-a-time:** the outer
   `shell #32k </DD/run.s` is an EXTRA #32k process on top of the four #32k
   basic09 racers, and run.s launches all four nearly simultaneously, so their
   `SHELL` forks coincide. One-at-a-time driving adds no such shell and staggers
   the launches, keeping peak RAM under the limit — which is the real reason it
   "cures" the flood.

So the flood is **memory exhaustion surfacing as a debugger EOF-spin** — NOT
channel corruption, NOT rapid `key`, NOT a backgrounded child eating the
procedure file, and crucially **NOT an RBF fault hiding behind it** (#237 is
benign to RBF). Every one of those earlier guesses is now disproven on demand.

**Ledger lesson, kept:** the previously-recorded cause ("channel corruption from
rapid key", and before that "a backgrounded child inherits the parent's stdin")
was written into three files WITHOUT being made to fail. Both were wrong. The
rule held: reproduce first, then name the cause.

**Standing harness hazard this exposes:** the workers have NO `ON ERROR` (only
the `read` role does), so ANY uncaught worker error — a real RBF error included —
manifests as this same hang-and-flood rather than a diagnosable message. A worker
`ON ERROR` that reports the code to `#2` and exits would turn a masked flood into
`worker N error NNN`. Not yet applied: the worker template is shared with the
68k target, whose results are proven against its current text, so changing it is
an owner decision.

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



### Destructive scenarios — the disk-image damage check (Task 8)

Every `.rbfImage` scenario now also runs `dcheck`/`free` and fails on
`StructuralOracle` damage (see Question 1), so a run that leaves the filesystem
cross-linked or leaking clusters is caught even when every file reads back clean.

| Destructive scenario | What RBF did | How it is kept honest |
|---|---|---|
| **disk full** — one worker writes 500 records to a `mount -k=16K` image | Raised `E_FULL` (248) at the wall — `hammer: worker 1 ERROR err 248`, `0 free sectors` — and `dcheck` certified the FULL image `file structure is intact`. RBF hit the limit without corrupting the disk. `Live` 2026-07-21 | The test REQUIRES `ERROR err 248` in the transcript, so a roster that quietly fit (a large device) fails the "was it exhausted?" guard rather than passing vacuously; and the structural oracle is proven to catch a real bitmap corruption |
| **delete held-open** — pre-extend a file, two readers walk it slowly (nap=3), `del` it mid-read | Deferred-delete, done right: both readers `read done 40` off the doomed file, then on last close the clusters were reclaimed (`free` back to the pristine 2012/2016) and `dcheck` stayed intact. `Live` 2026-07-21 | Two checks that can each fail: `dcheck` structural damage (oracle proven-failable), AND `free >= capacity-8` -- a delete-while-open that leaked the file's clusters would leave free well short and fail the reclamation assertion |
| **kill mid-write** — a lone slow writer (nap=3) is `kill 3`ed while extending a file | Clean process-death cleanup: proc 3 `Exited with Error #000:228 (E_PRCABT)` after ~5 of 100 records, `F$Exit` closed its open path, and `dcheck` reported the half-written file's device intact (9 of 2016 sectors used, accounted for). `Live` 2026-07-21 | The test REQUIRES the absence of `worker 1 wrote 100` -- a writer that finished before the kill fails the "was it interrupted?" guard -- plus the proven-failable structural oracle |
| **truncate** — a file is pre-extended to 200 records (~50 sectors) then shrunk to 320 bytes via `SS.Size` (the `trunc` helper) | The file really shrinks (reads back at 320 bytes) and `dcheck` stays intact, BUT the ~48 tail sectors are **not reclaimed** — free stays 1960/2016 instead of returning to ~2008. A live FINDING, below. `Live` 2026-07-21 | Three checks: the retrieved file must be exactly 320 bytes (a no-op trunc fails it), the proven-failable structural oracle, and `free >= capacity-8` wrapped in `XCTExpectFailure` — green today, flips loud when `pRsetsz` learns to free the tail |

### ★★ FIXED (was: delete-during-write leaks clusters)

**Resolution 2026-07-21:** fixed in `file_rbf.c` by matching canonical OS-9 —
`pRdelete` now refuses to delete a file another path holds open for write,
returning `E$Share` (253), the exact behaviour the NitrOS-9 cross-check below
proved the reference RBF uses. New helper `RingHasOtherWriter` walks the open
path ring (the same structure the shared-buffer cache uses) for any member with
`wMode` set. Verified live: the pre-fix binary orphaned 28 clusters with no
refusal; post-fix all four mid-write `del`s return `Error #000:253 (E_SHARE)`,
the writers finish, and `dcheck` reports the device intact.
`testDeletingFilesOpenForWriteIsRefusedWithShare` now guards it — a regression
that allowed the delete again drops the E_SHARE lines AND fires the orphan
oracle, both loud. Readers still delete-defer cleanly
(`testDeletingAFileHeldOpenLeavesTheImageIntact` unchanged) because the gate is
write-mode only. This also closes the ROADMAP "E$Share not enforced when
deleting an open file" item for the RBF path. Original finding, kept as the
record of how it was chased down:

The destructive hammer found a genuine, **deterministic, isolated** RBF
allocation fault in os9exec (68k):

**Deleting a file while it is still being WRITTEN orphans the clusters allocated
after the delete.** Four `append` workers grow their own files; part way in all
four are `del`eted; the workers keep writing (deferred delete) and finish. The
device then reports, every run:
- `dcheck`: many `Sector NNN ... not in file structure` -- clusters marked
  allocated that no file owns.
- `free`: short of pristine (1984 of 2016 vs the expected 2012) -- ~28 leaked.

It is **not** delete itself: `testLeakIsolation_deleteAfterClose` deletes the
same kind of `append`-grown file AFTER the writer closes and reclaims **perfectly
(2012/2016, `dcheck` intact)**. The only variable is whether writes continue
after the `del`. So the fault is specifically the post-delete writes' allocation
never being freed on close.

Harness exonerated: the isolation reclaims clean, the workers really did write
past the delete (`wrote 40`), and the structural oracle is proven to catch real
bitmap corruption.

**Root cause, from reading `Source/OS9exec_core/file_rbf.c` (read-only, not
changed):** `DeallocateBlocks` -- the only routine that frees a file's whole
segment list -- has exactly ONE caller: `Delete` (line ~3713). It walks the file
descriptor's segment list *as it stands at delete time* and releases those
clusters; **close never deallocates.** So when another open path keeps writing
after the `del`, it allocates fresh clusters into its own descriptor that
`DeallocateBlocks` has already run past and that no later close frees -- they
orphan. Delete-while-reading doesn't grow the file, so nothing new is allocated
and it reclaims clean, exactly as observed. Two plausible fixes, both the owner's
call: refuse to delete a file open for write (`E_SHARE`, as some OS-9 versions
do), or defer/rerun deallocation on the last close of a deleted file. Recorded
with `XCTExpectFailure` in `testDeletingFilesMidWriteReclaimsAllSpace`: the suite
stays green, and if os9exec is ever changed here the test "unexpectedly passes"
and fails loudly, prompting a revisit.

**★ Cross-checked on real NitrOS-9 (6809) -- it does NOT have the leak, because
it forbids the operation that causes it.** `Live` on a `format`ted 512-sector
RAM disk (`/r0`): a writer holds a file open `UPDATE`, another `del` is issued
mid-write, and NitrOS-9 RBF returns **`Error #253` (Non-sharable file busy,
`E$Share`)** -- it REFUSES to delete a file open non-sharable. Close the file
first, then `del` succeeds, `free` returns to baseline (495/495), `dcheck`
intact. So the reference RBF makes the leak impossible by rejecting delete of an
open write file. os9exec instead ALLOWS that delete (deferred) and then leaks the
post-delete writes. **This is the strongest evidence the os9exec behaviour is a
defect, not undefined-but-acceptable: the canonical RBF returns `E$Share` here,
and matching it (enforce the non-sharable check on the delete path) is the
clean fix.** (NitrOS-9's `dcheck` uses the same "file structure is intact" clean
bill, so the oracle ports; its `free` groups digits with commas on large
devices, which the oracle's regex would need to tolerate for a full 6809 port.)

### ★ FINDING (candidate): truncate does not reclaim the tail's clusters

The truncate scenario is now built — the 68k shell exposes no `SS.Size`, so a
tiny cio-free helper (`test/rbf-hammer/trunc`, source `trunc.c`) opens the file
and calls `_ss_size`. os9exec cannot fork a binary from the host-directory
scratch, so the harness copies it onto the RBF image and runs it there. It found
a second, independent allocation gap:

**os9exec's `SS.Size` shrink sets the logical size but never frees the truncated
tail's clusters.** Pre-extend `tr.dat` to 200 records, shrink to 320 bytes: the
file really shrinks (the retrieved copy is exactly 320 bytes) and `dcheck` stays
intact, but `free` stays 1960/2016 — the ~48 tail sectors remain allocated to
the file. They are reclaimed only when the file is later deleted, not on the
truncate.

**Root cause, from reading `file_rbf.c` (read-only, not changed):** the `SS.Size`
handler `pRsetsz` (~line 3995) does `Set_FDSize(spP,*size)` + `RingSetLastPos` +
`WriteFD` — it updates the logical size and the open-path ring, but never calls
`ReleaseBlocks`/`DeallocateBlocks` to return the now-unused tail segments to the
bitmap. So the space is retained by the file, not orphaned — which is exactly why
`dcheck` reports "file structure is intact": the clusters are still in `tr.dat`'s
own segment list. This is un-reclaimed space, NOT a cross-link or a tear.

Standard OS-9 RBF frees the tail on a shrink. Recorded with `XCTExpectFailure` in
`testTruncatingAFileShrinksItAndStaysIntact`: the suite is green today and flips
to a loud "unexpectedly passed" the moment `pRsetsz` learns to release the tail,
prompting removal of the wrapper. **Marked candidate, not confirmed, pending a
6809/NitrOS-9 cross-check** — the delete-leak above was only nailed as a defect
(not undefined-but-acceptable) once the reference RBF was shown to differ; the
same rigor is owed here before calling it a bug. Cross-check is harder than the
delete one: it needs an `SS.Size` path on 6809 (NitrOS-9's shell has no truncate
either, so a 6809 `trunc` equivalent or a Basic09 `SS.Size` call is required).

### ★★ FINDING (reproduced, CANDIDATE): SS.Size GROW discloses deleted data

The same `pRsetsz` block-list mishandling has a confidentiality face, and the
hammer reproduces it deterministically (`testGrowingAFileViaSetSizeDoesNotDiscloseDeletedData`,
68k, 0.4s): **a file grown with `SS.Size` reads back a DELETED file's contents.**
Fill a 16K image to `E_FULL` with a `create` worker (every cluster stamped
`---------- unwritten slot`), `del` it (frees the clusters, pattern still on the
media), `echo VICTIMHDR >victim.dat`, then `trunc victim.dat 12000` (`SS.Size`
grow). The retrieved victim is 12000 bytes: `VICTIMHDR` followed by the deleted
filler's `unwritten slot` records, verbatim (~4 copies). The grow neither zeroes
the new region nor bounds the read to the file's own allocation, so it surfaces
whatever the reclaimed clusters still held.

Pinned with `XCTExpectFailure` on the safe invariant (no deleted-data bytes in
the grown file), so the suite CATALOGS the disclosure rather than reporting
green. Marked CANDIDATE, not confirmed a defect: real OS-9 RBF may not zero
`SS.Size`-grown space either (see [[nitros9-runb-abort-sssize]] — a stale-sector
disclosure was already seen on the runb abort `SS.Size` path), so this could be
OS-9-faithful rather than an os9exec bug. The owner (a firsthand OS-9 author) can
say whether os9exec should diverge and zero grown space. Either way it is now a
recorded, reproducible disclosure the suite guards.

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
