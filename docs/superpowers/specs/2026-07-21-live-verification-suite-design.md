# Live-verification regression suite — design (68k scope)

Written 2026-07-21, revised same day to 68k-only scope. Implements the 68k
half of skills-plan item #3: wrap the ~86 orphan files from the 68k
live-verification pass (`test/68k-live-verification/`) into a runnable
PASS/FAIL suite, so verified behavior stops depending on nobody touching the
wrong file.

**The 6809 half is out of scope for this design entirely** — see "6809 is
deferred" below. Nothing here builds toward it, stubs it, or assumes its
shape.

## Problem

The 68k syscall/C-library audit pass produced ~86 `.a`/`.c`/`.bas`/`.sh`
files that each proved something true about `os9exec` at the time they were
written. None of them run as part of `make test` or any other gate. A
regression in, say, `F$Alarm`'s register contract would sit undetected until
someone happened to re-run the exact right file by hand.

Surveying the corpus turned up more structure than expected:

- **Two files are already complete, runnable repro scripts**
  (`bmode-repro.sh`, `pipe-abort-repro.sh`) — shell scripts that build a
  scratch environment, drive `os9exec` with piped/timed commands, and grep
  the transcript for a `PASS`/`FAIL` marker they print themselves.
- **A minority of the rest already print a genuine runtime PASS/FAIL
  marker** (measured: 15/28 `.a` + 6/26 `.bas` files). These are mechanical
  to wrap.
- **The majority print raw register/output dumps for a human to eyeball**
  against the doc, written when the call's behavior was still being
  reverse-engineered (e.g. `batch1-01.a`'s `F$ID` section: "register
  convention unknown, print d0/d1/d2 raw"). Wrapping these means *authoring
  a new expected-value assertion* per file, derived from the file's own
  comments plus the corresponding doc row's now-settled `Live` finding —
  closer to writing ~70 new small test oracles than "wiring up existing
  ones." Confirmed with the user this is in scope; it's the bulk of the
  actual work.
- **A subset are paired multi-process scenarios** run interactively in the
  original passes (lostupdate init/incrementer/verify, pipes
  producer/consumer, preempt hog/talker, plus the eoflock/recordlock family).
  These need real choreography (background one process, time the other,
  check the combined transcript), not just a compile+run+grep. Confirmed
  with the user these get full choreography too, not a documented-only stub.

## 6809 is deferred (not stubbed)

Two independent reasons, both raised by the user:

1. **Sequencing.** The 6809 side is currently under active, hot
  investigation by a concurrent session (an RBF lost-update root-cause hunt,
  touching `Source/OS9exec_core/fcalls.c`/`memstuff.c` and adding new
  `test/6809-live-verification/rl-*.bas` fixtures as recently as minutes
  before this was written). The 68k side, by contrast, is believed largely
  settled. Building 6809 regression coverage against a moving target is
  wasted work; finishing 68k first is strictly higher-value right now and
  carries zero harness-contention risk.
2. **Where should 6809 test tooling even live?** `os9exec-git_code` bills
  itself as "a 68000 emulator plus a reimplementation of the OS-9 kernel" —
  it is not the NitrOS-9 project. The 6809 test infrastructure that has
  already accumulated here (`RBFHammer6809Tests`, `tools/nitros9repl.sh`,
  the `rl-*.bas` fixtures) tests a *different*, third-party codebase
  (NitrOS-9, which has its own fork — see memory
  `nitros9-build-and-contribution-setup`) using this repo as its home. That
  precedent already exists and isn't being unwound here (it's mid-use by a
  concurrent session), but this new component doesn't have to compound it.
  When a NitrOS-9 kernel fix is eventually contributed upstream, only the
  changed module source goes to the community repo — never the test
  harness that found the bug. That mismatch (test tooling for repo A living
  in repo B) is worth resolving deliberately — possibly by giving 6809
  verification tooling a home in the NitrOS-9 fork itself — rather than by
  default because that's where the first pieces happened to land.

Conclusion: this design and the implementation plan that follows it cover
**68k only**. The 6809 half becomes its own future design, written once the
lost-update investigation has settled and the placement question above has
an answer.

## Non-goals

- Not a rewrite or "fixing" of the original files' content — the goal is
  regression protection for what they already proved, not re-litigating
  findings.
- Not touching `test/Sources/RBFHammerCore/Adapter68k.swift` — it's shaped
  around the hammer's `Scenario`/`WorkerSpec` worker-template model, not raw
  source files, and is currently mid-edit by a concurrent session's
  lost-update investigation. This suite is a new, separate component.
- No `--target` flag or platform abstraction of any kind. Building an
  extensibility seam for a 6809 backend that isn't designed yet is
  speculative — when that phase starts, `LiveVerify` gets extended (or a
  sibling executable gets written) against real requirements, not a guess
  made now.

## Architecture

### New Swift executable target in the existing `test/` package

Follows the `OS9Tests`/`RBFHammer` precedent: a plain `main.swift` with a
hand-rolled pass/fail counter and `print("PASS: ...")`/`print("FAIL: ...")`
output, filterable by name via a `CommandLine` argument — not XCTest, to
match house style (`OS9Tests` isn't XCTest either).

```
swift run --package-path test LiveVerify
```

`GNUmakefile` gets one new target, matching `test`/`test-notick`/`hammer`'s
existing precedent of literally testing `os9exec` itself:

```
live-verify: $(EXE)
	swift run --package-path test LiveVerify
```

### Manifest-driven, not ~86 hand-copied scripts

`test/live-verification-manifest.json`, decoded with Foundation's
`JSONDecoder` (no new dependency). One entry per test:

```json
{
  "id": "f-id-time-cmpnam",
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

### Execution

Extends the existing `os9()`-style helper pattern from `OS9Tests/main.swift`
(launch `os9exec` directly, pipe commands, capture stdout+stderr with a
timeout). Adds a staged/timed variant for `choreography` category (real
`Thread.sleep`/`DispatchQueue.asyncAfter` between writes to the process's
stdin pipe), the same technique `pipe-abort-repro.sh` already uses via host
shell `sleep` between piped writes.

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
   repro script wired in), run for real against `os9exec`, each proven to
   fail once (temporarily break the assertion, confirm FAIL, restore).
3. **Scale out**: work through the remaining ~76 68k files in batches
   mirroring the original audit's own batch numbering, authoring oracles as
   needed. Track progress the same way the original passes did (a running
   punch list), not as one atomic PR.

The implementation plan (next step) covers phase 1 and 2 in full; phase 3 is
sized but not written file-by-file in the plan — the manifest schema makes
each subsequent batch a repeatable, well-understood unit of work rather than
something that needs its own design.
