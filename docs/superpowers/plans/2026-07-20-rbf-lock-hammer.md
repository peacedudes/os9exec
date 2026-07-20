# RBF Lock & Integrity Hammer — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a scenario-driven hammer that proves os9exec's and NitrOS-9's
RBF survives many concurrent processes contending on one file, on every
storage backend, without losing data or corrupting the disk.

**Architecture:** Parameterized BASIC09 worker programs run inside a single
emulator instance (that is where lock contention lives); a Swift driver owns a
declarative scenario table, two transport adapters (os9exec and XRoar), and
four host-side oracles. Verification is done on the host in Swift after
extracting the produced file, so the portable guest-code surface stays tiny.

**Tech Stack:** Swift 5.9 (existing `test/` SwiftPM package), BASIC09, os9exec,
XRoar + NitrOS-9 via `tools/nitros9repl.sh`, ToolShed for image extraction.

Design doc: `docs/superpowers/specs/2026-07-20-rbf-lock-hammer-design.md`

## Global Constraints

- **New Swift files must be SwiftLint-clean.** Warnings are code smell; fix the
  code, never add a pragma or a config exemption. Do NOT reformat the existing
  `main.swift` (123 pre-existing violations) — out of scope.
- **Line length: soft 132, hard 160.** No force unwraps, no `try!`.
- **Doc comments (`///`) on every public and internal API.**
- **File headers** per user CLAUDE.md, naming the generating model.
- **Never write a system disk.** 68k: per-run scratch device keyed by PID.
  6809: clone the golden-master image directory, discard after.
- **Concurrent runs must not interact.** Two hammers, or a hammer and
  `make test`, must be able to run simultaneously.
- **6809 BASIC09 `INTEGER` is 16-bit** (max 32767). Sequence numbers and
  checksums must stay in range on both targets or use `REAL`. This is a real
  divergence, not a theoretical one.
- **OS-9 text lines end in CR (0x0D), not LF.**
- **Every oracle must be demonstrated failing at least once** before its green
  result is trusted. Four checks in this codebase have been found unable to
  fail. An oracle that has never gone red is not evidence.
- **Parallel 6809 instances need unique `NITROS9REPL_SESSION`,
  `NITROS9REPL_BECKER_PORT`, `NITROS9REPL_CHAN_PORT`,** and their own cloned
  image directory. Run them headless (`-ui null -ao null`); cap at 3 (host
  load average rule).

---

## File Structure

**Create:**
- `test/Sources/RBFHammerCore/Record.swift` — record format, encode/decode
- `test/Sources/RBFHammerCore/Verifier.swift` — content invariants
- `test/Sources/RBFHammerCore/Scenario.swift` — scenario + worker model, table
- `test/Sources/RBFHammerCore/Adapter.swift` — transport protocol, run result
- `test/Sources/RBFHammerCore/Adapter68k.swift` — os9exec transport
- `test/Sources/RBFHammerCore/Adapter6809.swift` — XRoar transport
- `test/Sources/RBFHammerCore/Oracles.swift` — dcheck/free/alloc/liveness
- `test/Sources/RBFHammerCore/Pool.swift` — parallel scenario runner
- `test/Sources/RBFHammer/main.swift` — CLI entry point
- `test/Tests/RBFHammerCoreTests/*.swift` — unit tests (no emulator needed)
- `test/rbf-hammer/hammer.bas` — the parameterized guest worker

**Modify:**
- `test/Package.swift` — add library target, executable target, test target
- `GNUmakefile` — add `test-locks` and `test-locks-soak` targets
- `tools/nitros9repl.sh:52` — parameterize `DISKDIR`

---

### Task 1: Record format and host-side verifier

The riskiest thing to get wrong, and the only part testable with no emulator.
Do it first and completely.

**Files:**
- Create: `test/Sources/RBFHammerCore/Record.swift`
- Create: `test/Sources/RBFHammerCore/Verifier.swift`
- Create: `test/Tests/RBFHammerCoreTests/VerifierTests.swift`
- Modify: `test/Package.swift`

**Interfaces:**
- Produces: `Record` (`worker: Int`, `sequence: Int`, `payload: String`,
  `checksum: Int`), `Record.width = 64`, `Record.encode() -> String`,
  `Record.decode(_ line: String) -> Record?`,
  `Verifier.verify(data: Data, expecting: [WorkerExpectation]) -> [Violation]`
- Consumes: nothing

**Record format.** Fixed 64 bytes, CR-terminated, so offsets are computable and
a torn record is detectable by length alone:

```
W%02d S%05d C%05d P<payload padded to 46 chars>\r
```

Worker id 0-99, sequence 0-99999, checksum 0-65535 (sum of payload bytes mod
65536 — fits 6809's 16-bit INTEGER when accumulated with MOD each step).

- [ ] **Step 1: Add the library and test targets to Package.swift**

```swift
// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "OS9Tests",
    platforms: [.macOS(.v14)],
    targets: [
        .executableTarget(name: "OS9Tests", path: "Sources/OS9Tests"),
        .target(name: "RBFHammerCore", path: "Sources/RBFHammerCore"),
        .executableTarget(name: "RBFHammer", dependencies: ["RBFHammerCore"],
                          path: "Sources/RBFHammer"),
        .testTarget(name: "RBFHammerCoreTests", dependencies: ["RBFHammerCore"],
                    path: "Tests/RBFHammerCoreTests")
    ]
)
```

- [ ] **Step 2: Write the failing tests**

```swift
import XCTest
@testable import RBFHammerCore

final class VerifierTests: XCTestCase {
    /// A record survives an encode/decode round trip unchanged.
    func testRecordRoundTrip() {
        let record = Record(worker: 7, sequence: 42, payload: "hello")
        let decoded = Record.decode(record.encode())
        XCTAssertEqual(decoded?.worker, 7)
        XCTAssertEqual(decoded?.sequence, 42)
        XCTAssertEqual(decoded?.payload, "hello")
    }

    /// Every record is exactly the fixed width, so a short tail means a torn write.
    func testEncodedRecordIsFixedWidth() {
        XCTAssertEqual(Record(worker: 1, sequence: 1, payload: "x").encode().count,
                       Record.width)
    }

    /// A clean run from two workers reports no violations.
    func testCleanRunHasNoViolations() {
        let data = Self.stream(workers: [1: 0..<10, 2: 0..<10])
        let expected = [WorkerExpectation(worker: 1, count: 10),
                        WorkerExpectation(worker: 2, count: 10)]
        XCTAssertTrue(Verifier.verify(data: data, expecting: expected).isEmpty)
    }

    /// A missing sequence number is a lost update — the defect this exists to catch.
    func testMissingSequenceIsReportedAsLostUpdate() {
        var records = Array(0..<10).filter { $0 != 4 }
            .map { Record(worker: 1, sequence: $0, payload: "p\($0)").encode() }
        let data = Data(records.joined().utf8)
        let violations = Verifier.verify(data: data,
                                         expecting: [WorkerExpectation(worker: 1, count: 10)])
        XCTAssertTrue(violations.contains { $0.kind == .sequenceGap })
    }

    /// A truncated final record is a torn write, not a short file.
    func testTruncatedTailIsReportedAsTornRecord() {
        var text = Record(worker: 1, sequence: 0, payload: "ok").encode()
        text += String(Record(worker: 1, sequence: 1, payload: "bad").encode().prefix(20))
        let violations = Verifier.verify(data: Data(text.utf8),
                                         expecting: [WorkerExpectation(worker: 1, count: 2)])
        XCTAssertTrue(violations.contains { $0.kind == .tornRecord })
    }

    /// A corrupted payload is caught by the checksum even though the framing is intact.
    func testCorruptedPayloadIsCaughtByChecksum() {
        var text = Record(worker: 1, sequence: 0, payload: "good").encode()
        text = text.replacingOccurrences(of: "Pgood", with: "Pbadd")
        let violations = Verifier.verify(data: Data(text.utf8),
                                         expecting: [WorkerExpectation(worker: 1, count: 1)])
        XCTAssertTrue(violations.contains { $0.kind == .checksumMismatch })
    }

    /// A record from a worker that never ran means cross-file or cross-process bleed.
    func testPhantomWriterIsReported() {
        let data = Data(Record(worker: 9, sequence: 0, payload: "x").encode().utf8)
        let violations = Verifier.verify(data: data,
                                         expecting: [WorkerExpectation(worker: 1, count: 0)])
        XCTAssertTrue(violations.contains { $0.kind == .phantomWriter })
    }

    /// Two records with the same worker and sequence mean a duplicated write.
    func testDuplicateRecordIsReported() {
        let one = Record(worker: 1, sequence: 3, payload: "x").encode()
        let violations = Verifier.verify(data: Data((one + one).utf8),
                                         expecting: [WorkerExpectation(worker: 1, count: 2)])
        XCTAssertTrue(violations.contains { $0.kind == .duplicate })
    }

    /// Builds a well-formed stream for the given workers and sequence ranges.
    private static func stream(workers: [Int: Range<Int>]) -> Data {
        let text = workers.sorted { $0.key < $1.key }.flatMap { worker, range in
            range.map { Record(worker: worker, sequence: $0, payload: "p\($0)").encode() }
        }.joined()
        return Data(text.utf8)
    }
}
```

- [ ] **Step 3: Run the tests to verify they fail**

Run: `swift test --package-path test --filter VerifierTests`
Expected: FAIL — `cannot find 'Record' in scope`.

- [ ] **Step 4: Implement `Record.swift`**

Fixed-width encode; `decode` returns `nil` for any line that is not exactly
`Record.width` bytes or whose fields do not parse. Checksum is
`payload.utf8.reduce(0) { ($0 + Int($1)) % 65536 }`.

- [ ] **Step 5: Implement `Verifier.swift`**

`Violation.Kind` cases: `.tornRecord`, `.sequenceGap`, `.duplicate`,
`.checksumMismatch`, `.phantomWriter`, `.countMismatch`. Walk the data in
`Record.width` strides; a trailing partial stride is `.tornRecord`. Group by
worker, then check contiguity, duplicates, checksum, and expected count.

- [ ] **Step 6: Run the tests to verify they pass**

Run: `swift test --package-path test --filter VerifierTests`
Expected: PASS, 8 tests.

- [ ] **Step 7: SwiftLint the new files**

Run: `cd test && swiftlint lint --quiet Sources/RBFHammerCore Tests`
Expected: no output. Fix the code, not the config, for anything reported.

- [ ] **Step 8: Commit**

```bash
git add test/Package.swift test/Sources/RBFHammerCore test/Tests
git commit -m "Tests: RBF hammer record format and host-side verifier"
```

---

### Task 2: The guest worker program

**Files:**
- Create: `test/rbf-hammer/hammer.bas`

**Interfaces:**
- Produces: a BASIC09 procedure `hammer` taking parameters
  `(id: INTEGER, role: STRING, file: STRING, count: INTEGER, nap: INTEGER)`.

Roles for this task: `append` and `read` only. Later tasks add the rest — a
worker that cannot yet append is not independently testable.

`nap` is the voluntary sleep in ticks between iterations, inserted **between a
read and its write** in update mode. That is the window that matters, and with
`-q` (tick off) it is the only thing that creates interleaving at all.

- [ ] **Step 1: Write `hammer.bas` with the `append` and `read` roles**

Records are built to exactly `Record.width` = 64 bytes including the trailing
CR. Checksum accumulates with `MOD 65536` every step so it never exceeds
6809's 16-bit INTEGER.

- [ ] **Step 2: Run one worker by hand against os9exec**

Run: `OS9DISK=$PWD/h0 ./os9exec shell` then drive `hammer` with `append`,
count 10, against a file on `/h5`.
Expected: a 640-byte file.

- [ ] **Step 3: Verify the bytes with the Task 1 verifier**

Feed the produced file to `Verifier.verify`. Expected: no violations.
**This is the first end-to-end proof that the guest and host agree on the
record format** — if they disagree, everything downstream is meaningless.

- [ ] **Step 4: Commit**

```bash
git add test/rbf-hammer/hammer.bas
git commit -m "Tests: parameterized BASIC09 hammer worker (append/read roles)"
```

---

### Task 3: The 68k adapter and a single-worker end-to-end scenario

**Files:**
- Create: `test/Sources/RBFHammerCore/Adapter.swift`
- Create: `test/Sources/RBFHammerCore/Adapter68k.swift`
- Create: `test/Sources/RBFHammerCore/Scenario.swift`
- Create: `test/Sources/RBFHammer/main.swift`

**Interfaces:**
- Consumes: `Record`, `Verifier` from Task 1
- Produces:
  - `protocol Adapter { func run(_ scenario: Scenario) throws -> RunResult }`
  - `struct RunResult { let transcript: String; let produced: [String: Data];
    let timedOut: Bool }`
  - `struct Scenario { let name: String; let backend: Backend;
    let workers: [WorkerSpec]; let tick: TickMode; let iterations: Int;
    let targets: Set<Target>; let tags: Set<String> }`
  - `enum Backend { case ramDisk, rbfImage, hostDirectory }`
  - `enum TickMode { case on, off }`
  - `enum Target { case os9exec68k, nitros9_6809 }`
  - `struct WorkerSpec { let id: Int; let role: Role; let file: String;
    let count: Int; let nap: Int }`

Isolation mirrors the existing suite: device root at
`NSTemporaryDirectory()/rbfhammer-<pid>-<uuid>`, emulator cwd set there so
`mount -k` images land there, removed afterward unless the run failed.

- [ ] **Step 1: Write a failing integration test**

A one-worker `append` scenario on the `hostDirectory` backend, asserting
`Verifier.verify` returns no violations and the file is the expected length.

- [ ] **Step 2: Run it, expect failure** (`Adapter68k` does not exist).

- [ ] **Step 3: Implement `Adapter68k`**

Spawn `os9exec` with `-r`, plus `-q` when `tick == .off`. Build the shell
script that forks each worker with `&` and waits. Read produced files back
from the host directory directly (that is the point of this backend).

- [ ] **Step 4: Run it, expect PASS.**

- [ ] **Step 5: SwiftLint clean, then commit**

```bash
git commit -m "Tests: RBF hammer 68k adapter and scenario model"
```

---

### Task 4: Multi-worker contention and the remaining roles

**Files:**
- Modify: `test/rbf-hammer/hammer.bas` (add `update`, `follow`, `hold`,
  `churn`, `truncate`, `remove`)
- Modify: `test/Sources/RBFHammerCore/Scenario.swift` (scenario table)

- [ ] **Step 1: Add the `update` role and a 4-worker lost-update scenario**

Four workers, each doing read-modify-write on the *same* records, with `nap`
between read and write. This is the scenario the old counter race got wrong.
It must be run with `tick: .on` and `tick: .off`.

- [ ] **Step 2: Prove the scenario can fail**

Run it against a binary built with the record lock disabled (comment out the
lock acquisition in `file_rbf.c`, build to a scratch binary, do NOT commit).
Expected: `.sequenceGap` or `.checksumMismatch` violations reported.
**If it passes against the broken binary, the scenario is worthless — fix it
before continuing.** This is the exact trap the previous counter race fell
into.

- [ ] **Step 3: Add `follow`, `hold`, `churn`, `truncate`, `remove` roles**

- [ ] **Step 4: Add scenarios with 8 and 16 workers across all three backends**

- [ ] **Step 5: Commit**

---

### Task 5: Structural, allocation, and liveness oracles

**Files:**
- Create: `test/Sources/RBFHammerCore/Oracles.swift`

**Interfaces:**
- Produces: `Oracles.structuralCheck(transcript:) -> [Violation]`,
  `Oracles.allocationRoundTrip(before:after:) -> [Violation]`,
  `Oracles.liveness(result:) -> [Violation]`

- [ ] **Step 1: Parse `free` and `dcheck` output into a comparable value**

- [ ] **Step 2: Add the allocation round-trip** — capture `free` before, delete
  everything the run created, capture `free` after, compare.

- [ ] **Step 3: Prove each oracle can fail.** Structural: hand a deliberately
  corrupted image (reuse `tools/fuzz-rbf-image.sh` to produce one). Allocation:
  skip the delete step and confirm it reports a leak. Liveness: run a scenario
  with a deliberate infinite `follow` and confirm the timeout is reported as a
  violation, not silently.

- [ ] **Step 4: If the allocation round-trip produces false failures**, RBF's
  allocator is not deterministic about which clusters it returns. Weaken to
  "same total free space and no unreachable clusters" and **record why in the
  design doc** — do not just delete the check.

- [ ] **Step 5: Commit**

---

### Task 6: Parallel scenario pool and the CLI

**Files:**
- Create: `test/Sources/RBFHammerCore/Pool.swift`
- Modify: `test/Sources/RBFHammer/main.swift`

CLI: `RBFHammer --target 68k|6809 --iterations N --jobs J [--gate]
[--keep-going] [--scenario NAME]`.

Default is stop at first failure and preserve the scratch device, printing its
path. `--keep-going` runs everything and reports a failure rate.

- [ ] **Step 1: Implement the pool with `withThrowingTaskGroup`, `J` concurrent
  scenarios, each fully isolated.**

- [ ] **Step 2: Verify two hammers run simultaneously without interacting** —
  launch two with different iteration counts, confirm both pass and neither
  scratch directory is touched by the other.

- [ ] **Step 3: Verify a hammer and `make test` run simultaneously.**

- [ ] **Step 4: Commit**

---

### Task 7: 6809 support — image cloning and unique instances

**Files:**
- Modify: `tools/nitros9repl.sh:52` (parameterize `DISKDIR`)
- Create: `test/Sources/RBFHammerCore/Adapter6809.swift`

- [ ] **Step 1: Parameterize `DISKDIR`**

```sh
DISKDIR="${NITROS9REPL_DISKDIR:-$NITROS9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker}"
```

Verify the default path still boots unchanged before going further.

- [ ] **Step 2: Clone the golden master per run.** Copy the whole image
  *directory* (the ROM lives there too, and XRoar loads it relative to cwd) to
  a per-run temp dir. Discard afterward unless the run failed.

- [ ] **Step 3: Verify two XRoar instances run concurrently** with distinct
  `NITROS9REPL_SESSION`, `NITROS9REPL_BECKER_PORT`, `NITROS9REPL_CHAN_PORT`,
  and cloned directories. Headless (`-ui null -ao null`).

- [ ] **Step 4: Confirm whether NitrOS-9 has a usable RAM disk descriptor.**
  If not, tag RAM-disk scenarios 68k-only rather than dropping them.

- [ ] **Step 5: Implement `Adapter6809`** — drive via `tools/b09run.sh` and
  `nitros9repl.sh`, extract produced files with ToolShed `os9 copy` from the
  cloned image.

- [ ] **Step 6: Run the Task 1 record round trip on 6809.** This is where
  BASIC09 dialect divergence will surface if it is going to. Expected: the
  same 64-byte records the 68k produces.

- [ ] **Step 7: Commit**

---

### Task 8: Destructive scenarios

**Files:**
- Modify: `test/Sources/RBFHammerCore/Scenario.swift`

Safe by construction — scratch device or discarded clone only.

- [ ] **Step 1: Kill a writer mid-write** (`remove` role plus a `kill`), assert
  the disk stays structurally clean and no cluster leaks.
- [ ] **Step 2: Exhaust free space** on a deliberately small device while
  readers follow.
- [ ] **Step 3: Delete a file while several processes hold it open.**
- [ ] **Step 4: Truncate under an active reader.**
- [ ] **Step 5: Confirm each produces a clean `dcheck` and a clean allocation
  round trip, or report the defect.**
- [ ] **Step 6: Commit**

---

### Task 9: Makefile targets, soak tuning, documentation

**Files:**
- Modify: `GNUmakefile`
- Create: `test/rbf-hammer/README.md`

- [ ] **Step 1: Add targets**

```make
test-locks:       ## fast deterministic lock gate (under a minute)
	swift run --package-path test RBFHammer --gate --target 68k

test-locks-soak:  ## long soak; override N and J
	swift run --package-path test RBFHammer --target 68k \
	    --iterations $(or $(N),2000) --jobs $(or $(J),4)
```

- [ ] **Step 2: Tune the gate to under 60 seconds.** Measure, do not guess.

- [ ] **Step 3: Tune the soak default to roughly 10 minutes per platform.**

- [ ] **Step 4: Run the full soak on both targets and record the results** in
  `test/68k-live-verification/` and `test/6809-live-verification/`, in the
  existing dogfood-report style, with the before/after framing an outside
  maintainer can follow.

- [ ] **Step 5: Confirm `make test` is still 132/0** with the tick on and with
  `make test-notick`.

- [ ] **Step 6: Commit**

---

## Self-Review

**Spec coverage.** Every design section maps to a task: layered gate/soak
(Task 6, 9), parameterized workers (Tasks 2, 4), declarative table (Tasks 3, 4),
two adapters (Tasks 3, 7), isolation (Tasks 3, 6, 7), four oracles (Tasks 1, 5),
all backends (Task 4), tick on/off (Tasks 3, 4), voluntary sleeps (Task 2, the
`nap` parameter), destructive testing (Task 8), golden master and clone
(Task 7), stop-vs-keep-going (Task 6), N as call-site parameter (Tasks 6, 9),
prove-it-can-fail (Tasks 1, 4, 5).

**Known open risks, deliberately not resolved on paper:**
1. **The allocation round-trip may produce false failures** if RBF's allocator
   is not deterministic. Task 5 Step 4 handles it explicitly rather than
   pretending it will not happen.
2. **The 6809 C toolchain is unverified.** The plan uses BASIC09 throughout, so
   nothing depends on C. If exact-offset unbuffered I/O turns out to be
   necessary, that is a new task, not a silent substitution.
3. **NitrOS-9 RAM disk availability is unverified** — Task 7 Step 4.
4. **Cross-instance sharing** (two emulators, one host directory) is deliberately
   NOT in the plan. It is a design question for the owner, not a test question.

**Type consistency.** `Record`, `Violation`, `WorkerExpectation`, `Scenario`,
`WorkerSpec`, `Backend`, `TickMode`, `Target`, `Adapter`, `RunResult` are used
with the same names and shapes in every task that references them.
