# RBF hammer — handoff prompt

Paste the block below to the next session.

---

Read project memory `rbf-lock-hammer-in-progress` first, then
`test/rbf-hammer/FAILABILITY.md`. Both are current as of 2026-07-20.

## Where things stand

A multiprocess RBF stress harness exists and works on 68k/os9exec. 14 scenario
tests + 16 unit tests, all green, each runnable standalone:

```sh
swift test --package-path test --filter ScenarioTests
swift test --package-path test --filter testTwelveWorkersShareOneFileOnRBFImage
RBF_HAMMER_RECORDS=200 swift test --package-path test --filter ScenarioTests
```

Architecture: one parameterized BASIC09 worker (`test/rbf-hammer/hammer.bas`,
a template the Swift driver substitutes into), a declarative scenario table,
transport adapters per target, and host-side oracles in Swift.
Design: `docs/superpowers/specs/2026-07-20-rbf-lock-hammer-design.md`.
Plan: `docs/superpowers/plans/2026-07-20-rbf-lock-hammer.md`.

**Record locking is proven working, not merely green.** An unlocked control
(`rmwfree` — read through a READ path, write through a WRITE path, so RBF's
update-mode auto-lock never engages) loses 80 of 100 updates. The locked
version keeps 100/100 with the tick on and off.

## Your task, in this order

### 1. Port the harness to 6809 / NitrOS-9 (plan Task 7)

This is the priority. The 68k side is done and the 6809 is the higher-stakes
target: NitrOS-9's RBF is a community-maintained clone, and a defect there can
destroy a real user's disk.

- Environment was FREE at handoff (no tmux server, no XRoar, becker port clear).
- `tools/nitros9repl.sh` already parameterizes `NITROS9REPL_SESSION`,
  `_BECKER_PORT`, `_CHAN_PORT`, so several XRoar instances can run at once.
  The one gap is `DISKDIR`, hardcoded at line 52 — parameterize it.
- **Golden master + clone per run, discard after** (the owner's explicit
  decision). Clone the whole image DIRECTORY, not just the `.ide` file: the ROM
  lives there and XRoar loads it relative to cwd. Never touch the boot disk.
- Run parallel instances headless (`-ui null -ao null`); cap ~3, host
  load-average rule.
- Parallel instances buy THROUGHPUT, not contention. Contention needs N OS-9
  processes inside ONE emulator. Do not confuse the two.
- Extract produced files from the cloned image with ToolShed `os9 copy`;
  verification stays host-side in Swift.

Verify on arrival, do not assume:
- **Does `SHELL` exist in 6809 BASIC09?** Documented only in the 68k-era
  manual; 6809 absence is UNCONFIRMED, not established. If missing, set
  `napMode` to `.nilWrites` (the pure-BASIC09 fallback, already implemented).
- Does NitrOS-9 have a usable RAM disk descriptor? If not, tag RAM-disk
  scenarios 68k-only rather than dropping them.
- 6809 BASIC09 `INTEGER` is SIGNED 16-bit. The record format already avoids
  `MOD 65536` for this reason — do not reintroduce it.
- NitrOS-9 has no `-q`; it is always pre-emptive, so the tick-off axis is
  68k-only. Voluntary naps are the portable interleaving mechanism.

**Divergence between 68k and 6809 is REPORTED, not failed.** Neither
implementation is an oracle for the other — that is a standing project rule.

### 2. Classify the N=200 / N=500 failures

Unclassified, NOT findings. `testTwelveWorkersShareOneFile` passes at N=500 in
isolation, so it is either other scenarios or an interaction. First suspect:
12 x 500 x 64 = 384000 bytes against a 500K image is tight, so disk-full may be
being tripped over accidentally. Isolate each one before calling it an RBF
defect.

### 3. Remaining plan tasks

Destructive scenarios (Task 8), the parallel soak pool and CLI (Task 6),
`PACK`+`runb` to stop re-parsing the worker on every launch (Task 6 Step 0),
Makefile targets and a tuned ~10-minute soak (Task 9).

## Rules this harness runs by — do not relax them

- **`test/rbf-hammer/FAILABILITY.md` is the gate.** Every check must be shown
  going RED before its green counts. A scenario with no ledger row is not
  coverage, whatever colour it prints. Four checks in this codebase have been
  found unable to fail, and the old `dogfood-lostupdate-*` race passed 600/600
  against code with no locking at all.
- **Prefer a control that changes the GUEST's behaviour over one that changes
  the emulator.** The `rmwfree` mode trick proved the lock in one run; the
  planned os9exec rebuild was never needed.
- **A red result does not accuse RBF until the harness is exonerated.** One
  failure this session looked exactly like "RBF loses data under concurrent
  writers" — intermittent, shared-files-only, RBF-image-only — and was the
  harness seeking past EOF. Running the worker ALONE exposed it.
- **Read the `os9-dev` skill before designing anything guest-side.** It is
  accurate and nearly complete, and every landmine hit this session was already
  documented in it.

## Known gaps in the harness itself

- `Adapter68k` never deletes its scratch directory. Deliberate for post-mortem,
  but it leaked 228 directories in one session. Add cleanup-on-success, keeping
  the directory only when a scenario fails.
- `slot` scenarios cannot detect a missing record lock (disjoint ranges never
  overlap). That is a coverage limit, not a bug — record-lock coverage comes
  from `rmw`/`rmwfree`. Both kinds are needed; neither substitutes.

## Housekeeping

- Several Claude sessions share this repo. `docs/nitros9-rbf-lock-*` is another
  session's in-flight work — leave it alone. Expect commits you did not make.
- Never clobber `./os9exec` or `build/*.o` when building a variant; other
  sessions use them. Build variants to a scratch path.
- `ROADMAP.md` is gitignored, local only.
- The `.gitignore` `h*` rule silently swallows any file starting with h or H.
