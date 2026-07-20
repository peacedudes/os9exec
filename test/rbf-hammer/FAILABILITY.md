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

### Negative result, recorded deliberately

| Claim | Mutation | Outcome |
|---|---|---|
| "Strict-ASCII decode in `Verifier` is required to catch garbage bytes" | Replaced `String(bytes:encoding:.ascii)` with lenient `String(decoding:as:UTF8.self)` | **Test still passed — the claim is not supported.** A replacement character corrupts the sequence field, the field parse fails, and `.tornRecord` is reported by a different route. Strict ASCII is retained on precision grounds (fails at the corruption, cannot merge several bad bytes into one replacement char) but **no test currently distinguishes the two implementations**, and `testGarbageByteIsReportedAsTornRecord` must not be cited as if it did. |

This row is the point of the ledger. The test looked like it guarded the
strict-ASCII choice, and it does not. Without the mutation check it would have
been quietly miscredited.

---

## Question 2 — scenario can produce bad data

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
