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
