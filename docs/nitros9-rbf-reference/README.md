# RBF reference module binaries

Known-good assembled RBF modules, so a new instance can confirm its build
before trusting any measurement. Check integrity with `tools/os9modcrc.py`.

> ## ⚠ 2026-07-26 — install `rbf.combined-eoflock-testdisk.mn`, not the older builds
>
> **Everything below dated 2026-07-20/21 gates the EOF lock on update mode.
> That is wrong.** Four Microware manuals state that a write landing at end of
> file gains EOF Lock in *any* write mode, that it is the only case where a
> write locks any part of a file, that it exists to stop two users extending a
> file at once, and that a sequential-output creator gains it on creation —
> which is what keeps a spooler one step behind an assembler writing through
> plain `>`. Cites: 6809 *System Programmers Manual* §6.6.1/§6.6.3/§6.6.5,
> Tandy *Technical Reference* :3799, Tandy *Level Two Development System*
> :12732, 68k *v2.4 Technical Reference* :6307. The designer has confirmed
> this as the intended contract.
>
> So the "defect" those builds fixed — a read-only reader following a
> write-only producer — **is the documented behaviour**. `rbf.patched*.mn` and
> `rbf.combined-fix*.mn` are kept only as the artifacts the 2026-07-20/21
> measurements were taken on. **Do not send them upstream.**
>
> See the current section at the end of this file.

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

## Current build (2026-07-26) — EOF lock for any write-capable path

Supersedes the combined build above. `docs/nitros9-rbf-lockmode.patch` was
re-cut to keep one of its three gates and correct a second:

| site | old gate | now | why |
|---|---|---|---|
| `L0BAA` RcdLock | `PD.MOD == UPDAT.` | **unchanged** | §6.6.1 restricts locking-on-*read* to update mode; that is what this site does. |
| `L0BF0` EofLock | `PD.MOD == UPDAT.` | `bita #WRITE.` | §6.6.3: a write at end of file gains EOF Lock in any write mode. A read-only path still asserts nothing — it can never extend the file, and a follower that locked the end would block the producer it is following. |
| `Creat131` | `PD.MOD == UPDAT.` | **gate removed** | §6.6.3: EOF Lock is gained as soon as a file is created for sequential output. Back to stock, which was right. |

Net +14 bytes over `main`'s 4846 (was +24). The patch applies on top of
`nitros9-rbf-lostupdate-regfix.patch`, not to bare `main` — its hunk offsets
assume that base, and the shipped module carries both.

| file | sha1 (first 8) | CRC | what it is |
|---|---|---|---|
| `rbf.combined-eoflock-testdisk.mn` | `94c4521f` | `$0C4B57` | Stock-on-disk + lostupdate regfix + re-cut lockmode + the 1-byte ChgDir accommodation. 4864 bytes. **The module the 2026-07-26 measurements were taken on — install this.** |
| `rbf.combined-eoflock.mn` | `01d3e0de` | `$BCF3BF` | Upstream `main` + both patches, nothing else — the PR artifact. Do not install on the test disk. |

### Measured live, 2026-07-26 (NitrOS-9 L2 V3.3.0, XRoar, private image)

| check | result |
|---|---|
| `rlsloww` — `WRITE` (`>`) producer | reader **follows**, 6 recs / 9s (×3) |
| `rlslowu` — `UPDATE` (`>+`) producer | reader **follows**, 6 recs / 9s (×2) |
| reader with no producer (falsification) | stops, 1 rec / 2s — so "follows" is a real signal |
| A/B/A | eof-fix follows 6/9s → old all-gates build stops 2/4s → eof-fix follows 6/9s again |
| record lock contention (`rlqhold`+`rlwait`) | waiter blocks (5s vs 2s uncontended) and reads the **post-write** value |
| lost update (`rlrace3` pair, `rlinit2`) | 400/400, `done=2`, **0 lost** (×2) |
| deadlock detection (`rldead1`/`rldead2`) | exactly one `E$DeadLk` #254; the other half completes |

Not yet covered: two write-only appenders extending one file concurrently —
the §6.6.3 purpose the EOF lock exists for. Under this build both assert the
lock, so they should serialize at the edge; the fixtures for it belong to the
6809 conformance suite's Phase 2 (`t11`), not here.

Pre-existing and untouched by this change: a reader at end of file gets
`Error #203 - Illegal Mode` rather than an EOF indication, on this build and
on stock alike.
