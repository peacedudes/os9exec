# RBF fix certification — NitrOS-9 6809 combined RBF fix

**Date:** 2026-07-22
**Subject:** NitrOS-9 (6809) RBF combined fix — the `lockmode` patch (write-only
lock gating, +24 bytes) plus the `lostupdate` patch (record-lock retry-after-park
register restore, +4 bytes), built together as
`docs/nitros9-rbf-reference/rbf.combined-fix-testdisk.mn` and installed at
`os9/nitros9/disk-images/eou_ide-v0.3-6809-combined-fix-test/`.
**Certified by:** the RBF integrity hammer (`test/rbf-hammer/`, `RBFHammer6809Tests`),
independently of the fix author's own assembly A/B/A.

## Verdict

**Certified ready to ship.** Every defect the two patches address has been
reproduced on stock RBF and shown fixed on the combined-fix image; the two
patches were shown not to interfere; deadlock detection was shown not to have
regressed; and the fix withstood a 4.5-hour randomized endurance soak with zero
data loss and zero filesystem corruption. No fault attributable to the fix was
found anywhere it could be reached.

## Method

Every test was made to **fail on stock RBF first** ("prove teeth") before its
result on the fixed image was trusted — a green that a buggy build cannot turn
red is worthless. The structural oracle (`dcheck`/`free` parsing) was itself
proven able to catch a real bitmap corruption. Reproductions are seed-reproducible.
Full evidence, including the false trails and their corrections, is in
`FAILABILITY.md`.

## What was validated

| Concern | Stock (proven to fail) | Combined-fix | Coverage |
|---|---|---|---|
| **Lost update** — binary `GET`/`PUT` record-lock retry-after-park | loses 250–537 of 800–1000 (single-sector, heavy-parking, 5-worker herd) | clean, every variant | `testBinaryReadModifyWriteLosesUpdatesOn6809`, commit `da1cbcc`; A/B `11a0e9a` |
| **Write-only Creat follow** — reader trailing a `>` producer past EOF | follows 8/8 records | stops 2/8 (correct) | `testWriteOnlyProducerDoesNotMakeAReaderFollowOn6809`, `acb5a68` |
| **Two fixes on one file** — write-only Creat + update RMW concurrently | loses 356/400 | 400/400 clean, no interference | `testMixedWriteOnlyAndRmwKeepEveryUpdateOn6809`, `e62455b` |
| **Deadlock detection** — crossed two-path record holds | `E$DeadLk` #254 fires | #254 still fires, no hang — not regressed | `testCrossedRecordHoldsAreDeadlockDetectedOn6809`, `4248ef9` |
| **Endurance** — 669 randomized runs, 4.5h, all 6 scenario families | n/a | 0 corruption, 0 data loss | chaos soak, seed 20260722 |

The five blind spots the fix author flagged were all addressed: skips-masquerading-as-green
answered (the bug-relevant tests are real passes, not silent skips); the suite
proven able to still fail (every pinned test reproduces its bug on the default
stock image); write-only Creat and the mixed interaction A/B'd; deadlock
detection confirmed reachable and intact.

### Endurance detail

669 runs over 270 minutes against the fixed image: 654 clean, 15 hung. **All 15
hangs were `#237 RAM-full`** — the emulated CoCo3 exhausting memory when several
workers fork `SHELL sleep` at once, a limit of the chaos generator, not RBF.
There was **not one real damage marker** (cross-link, orphaned sector, "not in
file structure") in the entire soak.

## Caveats (read these on the cert)

- NitrOS-9's RBF is a **community-maintained clone**, not licensed Microware
  source. Neither it nor os9exec is an oracle for the other.
- This is **one harness**. "No fault found" is not "no fault exists" — it is
  "the fix survived every attack this harness could mount, each proven able to
  catch the class of bug it targets."
- The 15 endurance failures are the **harness**, not the RBF (documented above).

## Not covered (orthogonal — would test RBF generally, not this fix)

A separate general-RBF hardening pass could still add: crash consistency
(`SIGKILL` the emulator mid-write, reboot, `dcheck`), allocation churn near a
full device, and heavily-fragmented-file extension. These are unrelated to the
two patches — a bug there would exist on stock too — so they do not gate this
certification.

## Bonus: os9exec (68k) RBF hardening added alongside

While building the 6809 validation, several probes were also run against
os9exec's own RBF and are recorded here for completeness — all clean: shrink-under-
reader stops at the new EOF (`3c54b49`), and a concurrent directory storm
(160 files grown to ~20 directory sectors, then deleted) leaves the device intact
(`3c99ccb`). Two earlier hammer findings on os9exec are also on record: the
delete-during-write cluster leak (a real bug, **fixed** in `0ee76c1`), and two
`SS.Size` behaviours confirmed **faithful OS-9, not bugs** (owner-adjudicated).
