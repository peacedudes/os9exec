# RBF reference module binaries

Known-good assembled RBF modules, so a new instance can confirm its build
before trusting any measurement. Check integrity with `tools/os9modcrc.py`.

| file | sha1 (first 8) | CRC | parity | what it is |
|---|---|---|---|---|
| `rbf.stock.mn` | `a64547c3` | `$245E32` | `$16` | Stock RBF **as it exists on the test disk**. The build reproduces this byte-for-byte, which is what validates the pipeline. |
| `rbf.patched-testdisk.mn` | `59f77e45` | `$514F09` | `$FF` | The module that produced the 2026-07-21 measurements. = stock-on-disk + all three lock-mode gates + the 1-byte ChgDir accommodation below. **Install this to reproduce.** |
| `rbf.patched.mn` | `0aa0a70c` | `$53BBB9` | `$FF` | The PR build: upstream `main` + `docs/nitros9-rbf-lockmode.patch`, nothing else. +24 bytes over `main`'s 4846. Ships in the PR; do not install on this test disk (see below). |

The patch gates all three places a lock could be asserted on `PD.MOD == UPDAT.`:
the `RcdLock` acquire (`L0BAA`), the `EofLock` acquire (`L0BF0`), and `Creat`'s
unconditional `EofLock` store at file creation (`Creat131`). By the design,
write-only (`>`) paths take no locks at all — only update paths do.

Earlier builds carrying only the first two gates (sha `d516767e` / `06f38b23`,
measured 2026-07-20) were superseded when the Creat site was shown live to leak
an eof lock to a write-only *creator* (reader follows `rlcreaw`; A/B/A'd).

## Why two patched builds

The test disk's RBF is one revision behind upstream `main`: its `ChgDir` tests
`EXEC.` where `main` tests `PEXEC.+EXEC.` — one byte, at module offset 782,
nothing to do with locking. To make the local build match the disk byte-for-byte
(so the only difference under test is the lock change), that one line is matched
locally. That accommodation is in `-testdisk` and is **not** part of the patch.

`rbf.patched.mn` is what the patch actually produces against `main`; it is the PR
artifact. It differs from `-testdisk` by exactly that one ChgDir byte (plus CRC).

## Reproduce

```sh
python3 tools/os9modcrc.py check docs/nitros9-rbf-reference/*.mn   # all ok
python3 tools/nitros9-install-rbf.py \
    docs/nitros9-rbf-reference/rbf.patched-testdisk.mn <path>/68IDE.ide
# boot, then (fixtures in test/6809-live-verification/):
#   rlsloww producer: reader stops     | rlslowu producer: reader follows
#   rlcreaw creator:  reader stops     | rlcreau creator:  reader follows
# revert with rbf.stock.mn to show both defects return (A/B/A).
```

## Lost-update fix (2026-07-21)

| file | sha1 (first 8) | CRC | what it is |
|---|---|---|---|
| `rbf.lostupdate-fix-testdisk.mn` | `8aaabd4c` | `$9EAFB1` | Stock-on-disk + `nitros9-rbf-lostupdate-regfix.patch` + the 1-byte ChgDir accommodation. **The module the 2026-07-21 A/B/A was measured on.** 4850 bytes. |
| `rbf.lostupdate-fix.mn` | `5874fb6e` | `$F2DA9C` | Upstream `main` + the same patch, nothing else — the PR artifact. Do not install on the test disk. |

The bug: the record-lock retry path (`L0B11`→`L0B1D`) reloaded only A from the
`L0B1B` stack frame; B held `L1053`'s `P$Signal` read (0 after a consumed
`S$Wake`) and X held the `F$Sleep`/`PE.TmOut` leftover (0).  Any lock request
under 256 bytes therefore re-presented as the count-0 "dismiss" request after a
park: the claim routine released everything, returned carry-clear, and the
woken waiter proceeded with **no lock**, racing the holder unlocked.  The fix
re-presents the caller's saved request (`ldd ,s` + `ldx 2,s`, one `bne`→`lbne`
for reach; +4 bytes).

Measured (rl-race3 pair, losses per 400, done=2-trusted runs):
stock 21/59/21/48/64 → fix 0/0/0/0 → stock again 110/73 → fix 0.
Under the fix: holder/waiter still blocks 3.4s and reads the post-write value;
crossed two-path holds still yield exactly one `E$DeadLk` #254 (same as stock);
the rl-race3o observation log shows 400/400 distinct reads (zero duplicates).

## Combined build (2026-07-21) — lockmode + lostupdate together, untested by hammer yet

Both patches applied to the same base, since both are headed upstream and had
only ever been A/B/A'd independently.

| file | sha1 (first 8) | CRC | what it is |
|---|---|---|---|
| `rbf.combined-fix-testdisk.mn` | `9704d7df` | `$8E59F9` | Stock-on-disk + `nitros9-rbf-lockmode.patch` + `nitros9-rbf-lostupdate-regfix.patch` + the 1-byte ChgDir accommodation. 4874 bytes (4846 + 24 + 4). |
| `rbf.combined-fix.mn` | `ddf06c86` | `$CB16A8` | Upstream `main` + both patches, nothing else — the PR artifact. Do not install on the test disk. |

Built from a fresh detached worktree at `origin/main` (`70c68f21`), confirmed
against the exact blob (`44a5479`) both patches were written against; both
applied with zero fuzz. Reviewed for interaction risk: the two patches touch
disjoint code regions (lockmode: `Creat131`, the RcdLock fast-claim path
around `L0BAA`, the `L0BF0` eof-lock assert; lostupdate: only the `L0B1D`
retry reload). Under lockmode's gating, a non-update path never reaches the
`L0B9F`/`L0B1D` claim-retry engine at all — it returns via `LokNoRc` before
ever contending — so lostupdate's fix, which only matters on that contended
retry path, is exercised identically to its standalone form for every case
that triggers it. No shared bytes, no shared branch targets.

Installed into a private disk copy
(`eou_ide-v0.3-6809-combined-fix-test/`, not in this repo) and **boot-verified
live**: clean `{N1|NN}` shell prompt, `mdir` confirms RBF resident. Functional
A/B/A on this combined build (lock-mode + lost-update fixtures together) not
yet run — that's the hammer session's next step, not done here.
