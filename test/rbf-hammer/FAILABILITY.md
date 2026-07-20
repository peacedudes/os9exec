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

**A genuine lock test needs OVERLAPPING read-modify-write on the same records**
— that is the missing scenario, and it is the next thing to build. Until it
exists and has its own row here, this harness has NO record-lock coverage,
regardless of how many green scenarios it reports.

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
