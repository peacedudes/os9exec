# 6809 conformance suite — Phase 2: record locking

> **For agentic workers:** this plan is executed with **deliberately lighter ceremony than Phase 1**. Read "How this plan is run" before starting.

**Goal:** Add three record-locking conformance tests (`t09`-`t11`) to the shipped 6809 suite, asserting RBF's documented automatic byte-range locking against what a real system does.

**Why this is the most valuable part of the suite:** RBF locks byte ranges with no call at all, purely as a side effect of ordinary reads and writes on an update-mode path. That is a documented *design promise*, not an implementation detail, and it is exactly the kind of claim a reimplementation can appear to honour while diverging under contention. Everything in Phase 1 is single-process; this is the part that needs a real machine.

**Spec:** `docs/superpowers/specs/2026-07-25-6809-selfhosted-conformance-suite-design.md`, "Phase 2 — record locking".

## How this plan is run

Phase 1 cost roughly as long as writing the original 99-file corpus. The rigour was right; applying it uniformly was not. For this plan:

- **Boot one guest and keep it.** Do not restart between tests or between steps. Check `tmux ls` and for existing `xroar` processes before booting anything — a live guest is reusable, and a stray one burns a full core beside yours.
- **Prove the mechanism once, not per test.** Phase 1 already established that the pipeline can report `FAIL` from a rebuilt image. For each new test, perturbing the expectation and observing FAIL **without** a full image rebuild is sufficient; one end-to-end rebuilt-image proof for the phase as a whole is enough.
- **Scale the review to the change.** A structural change gets a full review; a rewording gets an inline check. Do not spawn a re-review for a comment fix.
- All three tests may be built in **one dispatch** — they share fixtures and choreography, and splitting them would triple the guest setup cost.

## Global Constraints

Unchanged from Phase 1, and all still enforced by `verify-image.sh`:

- **Oracle policy.** Microware documentation is the standard; os9exec and NitrOS-9 are candidates for defect. **No test ships without a citation** — document and section — in `DOCS/claims.md`. A claim that cannot be pinned is dropped, not guessed.
- **No verbatim Microware text** anywhere that ships. The build fails on a double-quoted span longer than six words in `claims.md`.
- **Nothing runs privileged**; a test needing super reports `SKIP` with its reason.
- **Nothing outside the suite disk is written.** Fixtures live in `SCRATCH/` and are cleaned up on every path, including failure paths.
- Output line, byte-identical to the existing tests: `RESULT tNN VERDICT  obs=NNNNN exp=NNNNN  description`, five-digit zero-padded.
- Add every test to **all three** of `runall`, `runone`, `rebuild` — the build asserts they agree.
- CR-only text on the image; no non-ASCII; append with `>+` (`>>` is stderr).
- 6809 facts: user IDs are flat (not 68k `group.user`); `asm` labels are unique to 8 characters while `lwasm` accepts longer; `fcs "…"` for any string containing `/`.

## The two constraints that make these tests trustworthy

Both were paid for in the existing `rl-*` corpus and must carry over even though the files do not:

1. **Each racer self-reports completion.** Reading the shared counter alone cannot distinguish a lost update from a racer that had not finished yet, and the shell returns long before a backgrounded child is done — so every short read looks like data loss. A run counts only when every racer has declared itself finished.
2. **Every run proves its own preconditions.** A producer left over from an earlier run holding the data file open makes setup fail, leaves a stale file, and produces numbers that look exactly like results. Verify the fixture really was recreated by *this* run before believing any count.

**A timing-sensitive test that cannot establish its preconditions reports `ERROR`, never `FAIL`.** A fact about our harness must never be reported as a fact about the recipient's system.

---

### Task 1: `t09lost` — lost updates cannot occur

**Claim:** two processes each doing read-modify-write on the same record are serialized by the read's automatic lock and the write's release, so no update is lost.

**Files:** create `SRC/t09lost.bas` (+ packed module), `SRC/t09race.bas` (the racer child, +packed); modify `DOCS/claims.md`, `text/{runall,runone,rebuild}`.

- [ ] **Step 1:** Pin the citation before writing code. RBF automatic record locking is described in the OS-9 manuals at `/Users/rdoggett/mine/os9/XXX/os9exec/os9/txtResources`; the skill's `common/ipc.md` "Record locking" section is the finding aid, not the citation. If it cannot be pinned, report NEEDS_CONTEXT — do not guess.
- [ ] **Step 2:** Write `t09race.bas`: opens `SCRATCH/t09.dat` in **UPDATE** mode, loops N times doing GET / increment / PUT on record 1, and as its **last act under the same lock** increments a completion counter in the record. N=100 is enough; keep the run short.
- [ ] **Step 3:** Write `t09lost.bas`: creates the fixture, verifies it really was created by this run, forks two racers with `&`, waits for both, then reads the record. `obs` = the final counter; `exp` = 2*N. **Report `ERROR`, not `FAIL`, if the completion count is not 2** — that means a racer had not finished, which is a harness fact.
- [ ] **Step 4:** Pack both on the guest (`PACK` writes to the execution directory; a second pack of the same procedure in one session fails with error 51 — pack once per fresh session). Commit packed modules beside sources.
- [ ] **Step 5:** Run on the guest. Confirm PASS, then perturb `exp` and confirm a real `FAIL` line. No image rebuild needed for this step.
- [ ] **Step 6:** Commit — `Tests: 6809 conformance test t09 -- RBF prevents lost updates`.

### Task 2: `t10eoflk` — the EOF lock

**Claim:** a write landing at current EOF takes a lock where no data exists yet, so a reader that catches up waits at the edge instead of reading short; the real content stays readable throughout.

**Files:** create `SRC/t10eoflk.bas`, `SRC/t10prod.bas` (slow producer) + packed modules; modify `DOCS/claims.md` and the three runner files.

- [ ] **Step 1:** Pin the citation (same sources as Task 1; the EOF/"ghost" lock is the specific behaviour).
- [ ] **Step 2:** `t10prod.bas`: opens the fixture in UPDATE mode and appends a small known record every few ticks, a fixed number of times, then closes and sets a done-flag record.
- [ ] **Step 3:** `t10eoflk.bas`: creates the fixture, verifies its own precondition, backgrounds the producer, then reads forward. `obs` = records read before the producer's done-flag; `exp` = the number the producer wrote. A reader that raced past the edge reads short and `obs` differs.
- [ ] **Step 4:** Pack, run, confirm PASS, perturb and confirm FAIL, commit — `Tests: 6809 conformance test t10 -- the RBF EOF lock`.

**Note:** the user is the firsthand author of this design. If observed behaviour contradicts the manual, record both readings and ship the test — that disagreement is the finding.

### Task 3: `t11nolk` — only update-mode opens participate

**Claim:** read-only and write-only paths take no lock and wait on nothing. This is deliberate, so two independent appenders can interleave freely rather than excluding each other.

**Files:** create `SRC/t11nolk.bas`, `SRC/t11app.bas` (appender) + packed modules; modify `DOCS/claims.md` and the three runner files.

- [ ] **Step 1:** Pin the citation — specifically that write-only takes *no* lock, which is the part most likely to be quietly wrong in a reimplementation.
- [ ] **Step 2:** `t11app.bas`: opens the fixture **WRITE-only**, appends a known number of records, sets its own completion flag.
- [ ] **Step 3:** `t11nolk.bas`: backgrounds two appenders, waits for both completion flags, counts total records. `obs` = records present; `exp` = 2× the per-appender count. If either appender blocked, the run will not complete — bound the wait and report `ERROR` on timeout, never `FAIL`.
- [ ] **Step 4:** Pack, run, confirm PASS, perturb and confirm FAIL, commit — `Tests: 6809 conformance test t11 -- write-only paths take no lock`.

### Task 4: Phase close-out

- [ ] **Step 1:** One end-to-end proof for the phase: `make selfhost-6809` → `VERIFY-OK`, then a full `runall` from the **rebuilt image** on the guest showing 11/11, and a `rebuild` run showing the same.
- [ ] **Step 2:** Update `DOCS/ourruns.md` with the new baseline, still labelled a candidate observation rather than a standard.
- [ ] **Step 3:** Update `text/readme` only where the test count or behaviour description is now stale — it deliberately avoids naming counts, so this may be a no-op. Verify rather than assume.
- [ ] **Step 4:** Commit — `Docs: record-locking baseline for the 6809 conformance suite`.
