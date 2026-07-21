# Live-verification regression suite — design

Written 2026-07-21. Implements skills-plan item #3: wrap the ~140 orphan
files accumulated across the 68k and 6809 live-verification passes
(`test/68k-live-verification/`, `test/6809-live-verification/`) into a
runnable PASS/FAIL suite, so verified behavior stops depending on nobody
touching the wrong file.

## Problem

Two big verification passes (68k syscall/C-library audit, various dogfood
investigations on both platforms) produced ~140 `.a`/`.c`/`.bas`/`.sh` files
that each proved something true about `os9exec` or NitrOS-9 at the time they
were written. None of them run as part of `make test` or any other gate.
A regression in, say, `F$Alarm`'s register contract would sit undetected
until someone happened to re-run the exact right file by hand.

Surveying the corpus turned up more structure than expected:

- **Two files are already complete, runnable repro scripts**
  (`bmode-repro.sh`, `pipe-abort-repro.sh`) — shell scripts that build a
  scratch environment, drive `os9exec` with piped/timed commands, and grep
  the transcript for a `PASS`/`FAIL` marker they print themselves.
- **A minority of the rest already print a genuine runtime PASS/FAIL
  marker** (measured: 15/28 `.a` + 6/26 `.bas` files in the 68k directory).
  These are mechanical to wrap.
- **The majority print raw register/output dumps for a human to eyeball**
  against the doc, written when the call's behavior was still being
  reverse-engineered (e.g. `batch1-01.a`'s `F$ID` section: "register
  convention unknown, print d0/d1/d2 raw"). Wrapping these means *authoring
  a new expected-value assertion* per file, derived from the file's own
  comments plus the corresponding doc row's now-settled `Live` finding —
  closer to writing ~120 new small test oracles than "wiring up existing
  ones." Confirmed with the user this is in scope; it's the bulk of the
  actual work.
- **A subset are paired multi-process scenarios** run interactively in the
  original passes (eoflock reader/writer, lostupdate init/incrementer/verify,
  recordlock holder/waiter, pipes producer/consumer, preempt hog/talker).
  These need real choreography (background one process, time the other,
  check the combined transcript), not just a compile+run+grep. Confirmed
  with the user these get full choreography too, not a documented-only stub.

## Non-goals

- Not a rewrite or "fixing" of the original files' content — the goal is
  regression protection for what they already proved, not re-litigating
  findings.
- Not touching `test/Sources/RBFHammerCore/Adapter68k.swift` or
  `Adapter6809.swift` — both are shaped around the hammer's `Scenario`/
  `WorkerSpec` worker-template model, not raw source files, and `Adapter68k`
  is currently mid-edit by a concurrent session's lost-update investigation.
  This suite is a new, separate component.
- Not adding a `make` target for the 6809 half. `rl-runcell.sh` and
  `tools/nitros9repl.sh` already establish that NitrOS-9-specific tooling in
  this repo runs as a directly-invoked command, not through the (68k-focused)
  `GNUmakefile`. The 68k half *does* get a `make` target, matching
  `test`/`test-notick`/`hammer`'s existing precedent of literally testing
  `os9exec` itself.

## Architecture

### New Swift executable target(s) in the existing `test/` package

Follows the `OS9Tests`/`RBFHammer` precedent: a plain `main.swift` with a
hand-rolled pass/fail counter and `print("PASS: ...")`/`print("FAIL: ...")`
output, filterable by name via a `CommandLine` argument — not XCTest, to
match house style (`OS9Tests` isn't XCTest either).

Two invocations of one executable, `LiveVerify`, matching `RBFHammer`'s own
`--target 68k`/`--target 6809` convention:

```
swift run --package-path test LiveVerify --target 68k
swift run --package-path test LiveVerify --target 6809
```

`GNUmakefile` gets one new target for the 68k half only:

```
live-verify: $(EXE)
	swift run --package-path test LiveVerify --target 68k
```

### Manifest-driven, not 140 hand-copied scripts

`test/live-verification-manifest.json`, decoded with Foundation's
`JSONDecoder` (no new dependency). One entry per test:

```json
{
  "id": "f-id-time-cmpnam",
  "target": "68k",
  "kind": "asm",
  "sources": ["batch1-01.a"],
  "category": "solo",
  "expect": [
    {"contains": "PASS F$Time year-byte="},
    {"matches": "F\\$ID raw d0/d1/d2: [0-9A-F]{4} 00[0-9A-F]{6} 0000"}
  ]
}
```

- `kind`: `asm` | `c` | `bas` — selects the build/run recipe (assemble via
  `r68`/`asm`+`l68`, or `cc`, or hand the `.bas` straight to `basic09`).
- `category: "solo"` — compile/assemble once, run once, check `expect`
  against the transcript. `expect` is a list and **all** entries must match
  (AND, not OR) — a test with two assertions fails if either one doesn't
  hold, never passing on a partial match. Covers the self-checking files
  (`contains` patterns matching their own printed marker) and the raw-dump
  files once given an authored `matches` regex encoding the expected value.
- `category: "choreography"` — an added `roles` array, each `{source,
  delayBeforeStart, background}`, executed against one shared transcript;
  `expect` applies to the merged output. Covers the paired scenarios.
- A `notes` string field carries provenance (which doc row / dogfood report
  this test guards), so a future reader can trace a failure back to the
  original finding without re-deriving it.

### 68k execution

Extends the existing `os9()`-style helper pattern from `OS9Tests/main.swift`
(launch `os9exec` directly, pipe commands, capture stdout+stderr with a
timeout). Adds a staged/timed variant for `choreography` category (real
`Thread.sleep`/`DispatchQueue.asyncAfter` between writes to the process's
stdin pipe), the same technique `pipe-abort-repro.sh` already uses via host
shell `sleep` between piped writes.

### 6809 execution

Shells out to `tools/nitros9repl.sh` (`start`/`send`/`key`/`stop`), the same
way `rl-runcell.sh` already does, reusing its existing clone-isolation
environment variables (`NITROS9REPL_DISKDIR`/`_SESSION`/`_BECKER_PORT`/
`_CHAN_PORT`) so a private instance boots per run without touching whatever
instance a concurrent session has open. Does not import `RBFHammerCore`.

## Error handling

- A compile/assemble failure is always FAIL, reported distinctly ("build
  failed" vs. "assertion failed") so a broken toolchain doesn't read as a
  behavior regression.
- A timeout is reported as its own outcome (matching `OS9Tests`'
  `"(timeout)"` convention) — a hang is a different defect class than a
  wrong value and must not silently count as FAIL-with-no-explanation.
- Every manifest entry's `expect` patterns are validated (non-empty, valid
  regex where `matches` is used) by a fast unit-test pass with **no**
  emulator involved, before any run — a typo'd oracle should fail loudly at
  review time, not silently pass because it matches everything.

## Testing strategy (the runner itself)

Manifest parsing, pattern matching (`contains`/`matches`), and choreography
recipe validation are unit-tested against fixture JSON + fake transcript
strings — no emulator needed, fast, and (per this project's standing rule)
each check gets a deliberately-broken counterpart proving it can fail before
being trusted. This mirrors `tools/check_doc_consistency.py`'s own testing
discipline.

## Phasing

This is a multi-session build, not a single pass. Order:

1. **Infrastructure**: `LiveVerify` executable skeleton, manifest schema +
   decoder + pattern-matcher, unit tests for all of the above (no emulator
   runs yet).
2. **Proof-of-concept batch**: 5-10 manifest entries spanning every shape
   (solo/asm, solo/c, solo/bas, one choreography pair, one already-existing
   repro script wired in), run for real against `os9exec`/NitrOS-9, each
   proven to fail once (temporarily break the assertion, confirm FAIL,
   restore).
3. **Scale out**: work through the remaining ~130 files in batches mirroring
   the original audit's own batch numbering, authoring oracles as needed.
   Track progress the same way the original passes did (a running punch
   list), not as one atomic PR.

The implementation plan (next step) covers phase 1 and 2 in full; phase 3 is
sized but not written file-by-file in the plan — the manifest schema makes
each subsequent batch a repeatable, well-understood unit of work rather than
something that needs its own design.
