# DriveWire Virtual Serial + NitrOS-9 REPL Implementation Plan

> **SUPERSEDED 2026-07-25 — do not follow this.** The DriveWire fork described
> here no longer exists. Upstream implemented the same capability itself via the
> canonical `tcp connect|listen|join|kill` command API — the vocabulary
> NitrOS-9's own `inetd` and `lib/net.as` have always spoken — so the 6809 REPL
> now runs on **stock** drpitre/drivewire `main` with no patch at all. PRs #6
> and #7 are closed; both branches are deleted locally and on the fork (the
> commits survive as `refs/pull/{6,7}/head` upstream).
>
> Kept for the protocol findings, which still hold: OP_DWINIT must be answered
> with a non-zero byte, and a SERWRITEM to an unopened channel carries no count
> byte. For the current setup see `tools/nitros9repl.sh` and the README.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement DriveWire virtual serial channels in DrPitre's Swift host + a becker-port TCP listener, so a scripted REPL (`tools/nitros9repl.sh`) can drive a live NitrOS-9 (6809) shell under XRoar unattended.

**Architecture:** Three layers — (1) a `VirtualChannel` engine inside `DriveWireHost` replacing the OP_SER* stubs, implemented exactly per the repo's own `DriveWire Specification.md`; (2) a new `DriveWireTCPServerDriver` that listens on the becker port (65504) for XRoar and on `channelPortBase+N` per channel for REPL/human clients; (3) a tmux+nc REPL script in *this* repo mirroring `os9repl.sh`'s interface. Guest side needs no new code: the EOU `6809_ide_xroar_dw_becker` disk bakes `/N1../N4` + `dwio_becker` into OS9Boot.

**Tech Stack:** Swift (Xcode project, `xcodebuild`), Network.framework (`NWListener`), XCTest, ToolShed (`os9` CLI), `dd`, XRoar, tmux, `nc`.

## Global Constraints

- Swift repo: `/Users/rdoggett/mine/os9/XXX/os9exec/os9/nitros9/drivewire/drpitre-drivewire` — work on branch `virtual-serial`; **PR-quality**: match DrPitre's existing style (classes + delegate callbacks + `///` docs, NOT actors), every public/internal API gets a doc comment.
- **Never push or open a PR** — public actions need explicit user sign-off. Local commits only.
- Xcode project uses explicit pbxproj file lists — every new .swift file must be added to `project.pbxproj` by hand (pattern after `DriveWireTCPDriver.swift`'s entries).
- Build/test command: `xcodebuild -project swift/DriveWire.xcodeproj -scheme DriveWire test` (schemes: `DriveWire`, `drivewire-cli`). Some pre-existing tests reference `/Users/boisy/test.dsk` and fail on this machine — **that's pre-existing; only the new serial tests must pass**; use `-only-testing:` filters.
- REPL script lives in *this* repo (`os9exec-git_code/tools/`), commit style `Prefix: short description`, single line, stage files explicitly.
- Disk images: ALWAYS back up before an in-place `dd` write; `conv=notrunc` on every write-back; the pre-fix and post-fix backups get distinct, self-describing names.
- Spec source of truth: `drpitre-drivewire/DriveWire Specification.md` (serial section ~lines 360–540).
- tmux session name for the new REPL: `nitros9repl` (must not collide with `os9repl.sh`'s).

---

### Task 1: Branch + channel lifecycle (SERINIT/SERTERM/SERSETSTAT/SERGETSTAT)

**Files:**
- Create: `swift/DriveWire/Model/VirtualChannel.swift`
- Modify: `swift/DriveWire/Model/DriveWireHost.swift` (delegate protocol ~line 13; stub processors ~lines 854–908; state properties near `virtualDrives` ~line 92)
- Modify: `swift/DriveWire.xcodeproj/project.pbxproj` (add VirtualChannel.swift to DriveWire, drivewire-cli, DriveWireTests targets)
- Test: `swift/DriveWireTests/DriveWireTests.swift`

**Interfaces:**
- Produces: `public class VirtualChannel { public let channelNumber: UInt8; public private(set) var isOpen: Bool; var inputQueue: Data; func open(); func close() }`
- Produces: `DriveWireHost.virtualChannels: [VirtualChannel]` (15 entries, index = channel number)
- Produces: `DriveWireDelegate.channelDataAvailable(host:channel:data:)` with default no-op (used by Tasks 2–4)
- Produces: private helper `vserialChannel(_ number: UInt8) -> VirtualChannel?` (nil for ≥15, i.e. VWindow range unsupported)

- [ ] **Step 1: Create branch**

```bash
git -C /Users/rdoggett/mine/os9/XXX/os9exec/os9/nitros9/drivewire/drpitre-drivewire checkout -b virtual-serial
```

- [ ] **Step 2: Write failing tests** — append inside `DriveWireSwiftTests`:

```swift
    func testSerialChannelOpensAndCloses() throws {
        var open = Data([host!.OPSERINIT, 1])
        host!.send(data: &open)
        XCTAssertTrue(host!.virtualChannels[1].isOpen)
        var close = Data([host!.OPSERTERM, 1])
        host!.send(data: &close)
        XCTAssertFalse(host!.virtualChannels[1].isOpen)
    }

    func testSerialSetStatOpensAndCloses() throws {
        var open = Data([host!.OPSERSETSTAT, 2, 0x29])
        host!.send(data: &open)
        XCTAssertTrue(host!.virtualChannels[2].isOpen)
        var close = Data([host!.OPSERSETSTAT, 2, 0x2A])
        host!.send(data: &close)
        XCTAssertFalse(host!.virtualChannels[2].isOpen)
    }

    func testSerialSetStatComStConsumes29Bytes() throws {
        // SS.ComSt carries a 26-byte device descriptor. If the processor
        // miscounts, the trailing SERINIT below lands mid-stream and channel 3
        // never opens -- this asserts the stream stays in sync.
        var payload = Data([host!.OPSERSETSTAT, 0, 0x28])
        payload.append(Data(repeating: 0xAA, count: 26))
        payload.append(Data([host!.OPSERINIT, 3]))
        host!.send(data: &payload)
        XCTAssertTrue(host!.virtualChannels[3].isOpen)
    }
```

- [ ] **Step 3: Run tests, verify they fail** (no `virtualChannels` yet → compile error is the failure)

```bash
cd /Users/rdoggett/mine/os9/XXX/os9exec/os9/nitros9/drivewire/drpitre-drivewire
xcodebuild -project swift/DriveWire.xcodeproj -scheme DriveWire build-for-testing 2>&1 | tail -20
```
Expected: compile error `value of type 'DriveWireHost' has no member 'virtualChannels'`.

- [ ] **Step 4: Create `swift/DriveWire/Model/VirtualChannel.swift`**

```swift
//
//  VirtualChannel.swift
//  DriveWire
//

import Foundation

/// A virtual serial channel between the guest and the host.
///
/// Channels carry byte streams between guest-side devices (`/N1`...) and a
/// host-side consumer such as a TCP bridge. The guest opens and closes
/// channels with OP_SERINIT/OP_SERTERM or OP_SERSETSTAT SS.Open/SS.Close.
public class VirtualChannel {
    /// The channel number (0-14).
    public let channelNumber: UInt8

    /// Whether the guest currently has this channel open.
    public private(set) var isOpen = false

    /// Bytes waiting for the guest to collect via OP_SERREAD/OP_SERREADM.
    var inputQueue = Data()

    init(channelNumber: UInt8) {
        self.channelNumber = channelNumber
    }

    /// Marks the channel open and discards any stale queued input.
    func open() {
        isOpen = true
        inputQueue.removeAll()
    }

    /// Marks the channel closed and discards any queued input.
    func close() {
        isOpen = false
        inputQueue.removeAll()
    }
}
```

- [ ] **Step 5: Add pbxproj entries.** In `project.pbxproj`, mirror the four kinds of entries `DriveWireHost.swift` has (it's in all 3 targets). Generate three fresh 24-hex-digit IDs for the build files and one for the file reference (e.g. take `DriveWireHost.swift`'s IDs and change the last 6 digits; verify uniqueness with grep). Add:
  1. Three `PBXBuildFile` lines (sections near line 12).
  2. One `PBXFileReference` line (near line 59).
  3. One child entry in the same group that lists `DriveWireHost.swift` (near line 179).
  4. One entry in each of the three `Sources` build phase lists that contain `DriveWireHost.swift in Sources`.

- [ ] **Step 6: Implement in `DriveWireHost.swift`.**

(a) Extend the delegate protocol (after `transactionCompleted`):

```swift
    /// Informs the delegate that a virtual serial channel has data from the guest.
    ///
    /// - Parameters:
    ///     - host: The DriveWire host object.
    ///     - channel: The virtual serial channel number (0-14).
    ///     - data: The bytes the guest wrote to the channel.
    func channelDataAvailable(host : DriveWireHost, channel : UInt8, data : Data)
```

and directly below the protocol, a default so existing conformers compile unchanged:

```swift
public extension DriveWireDelegate {
    func channelDataAvailable(host : DriveWireHost, channel : UInt8, data : Data) {}
}
```

(b) State, next to `virtualDrives`:

```swift
    /// The virtual serial channels, indexed by channel number (0-14).
    ///
    /// Channel state is runtime-only and deliberately not part of the
    /// document's `Codable` representation.
    public var virtualChannels : [VirtualChannel] = (0..<15).map { VirtualChannel(channelNumber: UInt8($0)) }
```

(c) Private helper (near the OP_SER* processors):

```swift
    /// Returns the virtual serial channel for a wire channel number.
    ///
    /// Returns `nil` for numbers outside 0-14; the 128-142 range is the
    /// unimplemented VWindow space.
    private func vserialChannel(_ number : UInt8) -> VirtualChannel? {
        guard Int(number) < virtualChannels.count else { return nil }
        return virtualChannels[Int(number)]
    }
```

(d) Replace the four lifecycle stubs:

```swift
    private func OP_SERINIT(data : Data) -> Int {
        let expectedCount = 2
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERINIT
        resetState()
        vserialChannel(data[1])?.open()
        log = log + "OP_SERINIT(\(data[1]))" + "\n"
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }

    private func OP_SERTERM(data : Data) -> Int {
        let expectedCount = 2
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERTERM
        resetState()
        vserialChannel(data[1])?.close()
        log = log + "OP_SERTERM(\(data[1]))" + "\n"
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }

    private func OP_SERGETSTAT(data : Data) -> Int {
        let expectedCount = 3
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERGETSTAT
        resetState()
        // Logging only, per the specification.
        log = log + "OP_SERGETSTAT(\(data[1]),\(data[2]))" + "\n"
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }

    private func OP_SERSETSTAT(data : Data) -> Int {
        let ssComSt : UInt8 = 0x28, ssOpen : UInt8 = 0x29, ssClose : UInt8 = 0x2A
        guard data.count >= 3 else { return 0 }
        // SS.ComSt carries a 26-byte device descriptor after the code byte.
        let expectedCount = data[2] == ssComSt ? 29 : 3
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERSETSTAT
        resetState()
        switch data[2] {
        case ssOpen:
            vserialChannel(data[1])?.open()
        case ssClose:
            vserialChannel(data[1])?.close()
        default:
            break
        }
        log = log + "OP_SERSETSTAT(\(data[1]),\(data[2]))" + "\n"
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }
```

- [ ] **Step 7: Run the new tests**

```bash
xcodebuild -project swift/DriveWire.xcodeproj -scheme DriveWire test \
  -only-testing:DriveWireTests/DriveWireSwiftTests/testSerialChannelOpensAndCloses \
  -only-testing:DriveWireTests/DriveWireSwiftTests/testSerialSetStatOpensAndCloses \
  -only-testing:DriveWireTests/DriveWireSwiftTests/testSerialSetStatComStConsumes29Bytes \
  2>&1 | tail -5
```
Expected: `** TEST SUCCEEDED **`

- [ ] **Step 8: Commit**

```bash
git add swift/DriveWire/Model/VirtualChannel.swift swift/DriveWire/Model/DriveWireHost.swift swift/DriveWire.xcodeproj/project.pbxproj swift/DriveWireTests/DriveWireTests.swift
git commit -m "Implement virtual serial channel lifecycle (SERINIT/SERTERM/SERSETSTAT/SERGETSTAT)"
```

---

### Task 2: Guest→host writes (SERWRITE / SERWRITEM / FASTWRITE)

**Files:**
- Modify: `swift/DriveWire/Model/DriveWireHost.swift` (stubs `OP_SERWRITE`, `OP_SERWRITEM`, `OP_FASTWRITE_Serial`, `OP_FASTWRITE_Screen`)
- Test: `swift/DriveWireTests/DriveWireTests.swift`

**Interfaces:**
- Consumes: `vserialChannel(_:)`, `channelDataAvailable` (Task 1)
- Produces: private `deliverGuestBytes(_ bytes: Data, channel: UInt8)` — routes guest writes to the delegate; drops (with log) if channel closed/invalid.

- [ ] **Step 1: Add delegate capture + failing tests.** In the test class add the property and conformance:

```swift
    var channelData : [UInt8: Data] = [:]

    func channelDataAvailable(host: DriveWireHost, channel: UInt8, data: Data) {
        channelData[channel, default: Data()].append(data)
    }
```

and the tests:

```swift
    func testSerialWriteDeliversByte() throws {
        var s = Data([host!.OPSERINIT, 1, host!.OPSERWRITE, 1, 0x42])
        host!.send(data: &s)
        XCTAssertEqual(channelData[1], Data([0x42]))
    }

    func testSerialWriteMultipleDeliversBytes() throws {
        var s = Data([host!.OPSERINIT, 5, host!.OPSERWRITEM, 5, 3, 0x41, 0x42, 0x43])
        host!.send(data: &s)
        XCTAssertEqual(channelData[5], Data([0x41, 0x42, 0x43]))
    }

    func testFastwriteDeliversByte() throws {
        var s = Data([host!.OPSERINIT, 0, 0x80, 0x43])
        host!.send(data: &s)
        XCTAssertEqual(channelData[0], Data([0x43]))
    }

    func testFastwriteSplitAcrossSends() throws {
        // The opcode and its operand arriving in separate reads must not desync.
        var first = Data([host!.OPSERINIT, 0, 0x80])
        host!.send(data: &first)
        var second = Data([0x44])
        host!.send(data: &second)
        XCTAssertEqual(channelData[0], Data([0x44]))
    }

    func testWriteToClosedChannelIsDropped() throws {
        var s = Data([host!.OPSERWRITE, 9, 0x42])
        host!.send(data: &s)
        XCTAssertNil(channelData[9])
    }
```

- [ ] **Step 2: Run, verify failure** (same `-only-testing:` pattern, new test names). Expected: assertions fail (stubs deliver nothing).

- [ ] **Step 3: Implement.** Add helper + replace stubs:

```swift
    /// Routes bytes the guest wrote to a channel out to the delegate.
    private func deliverGuestBytes(_ bytes : Data, channel : UInt8) {
        guard let ch = vserialChannel(channel), ch.isOpen else {
            log = log + "serial write to closed or invalid channel \(channel)" + "\n"
            return
        }
        delegate?.channelDataAvailable(host: self, channel: channel, data: bytes)
    }

    private func OP_SERWRITE(data : Data) -> Int {
        let expectedCount = 3
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERWRITE
        resetState()
        deliverGuestBytes(Data([data[2]]), channel: data[1])
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }

    private func OP_SERWRITEM(data : Data) -> Int {
        guard data.count >= 3 else { return 0 }
        let expectedCount = 3 + Int(data[2])
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERWRITEM
        resetState()
        deliverGuestBytes(data.subdata(in: 3..<expectedCount), channel: data[1])
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }

    private func OP_FASTWRITE_Serial(data : Data) -> Int {
        let expectedCount = 2
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = data[0]
        resetState()
        deliverGuestBytes(Data([data[1]]), channel: fastwriteChannel)
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }

    private func OP_FASTWRITE_Screen(data : Data) -> Int {
        let expectedCount = 2
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = data[0]
        resetState()
        // VWindow channels are not implemented; consume correctly and log.
        log = log + "OP_FASTWRITE_Screen(\(fastwriteChannel)) unsupported" + "\n"
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }
```

**Note:** the existing `OP_OPCODE` dispatch (line ~1406) already routes $80–$8E to `OP_FASTWRITE_Serial` and sets `fastwriteChannel`; a 0 return correctly re-enters through `OP_OPCODE` when the operand arrives because `processor` is only reassigned via `dwTransaction` matches — verify `testFastwriteSplitAcrossSends` proves this.

- [ ] **Step 4: Run new tests — expect `** TEST SUCCEEDED **`. Also re-run Task 1's tests (regression).**

- [ ] **Step 5: Commit**

```bash
git add swift/DriveWire/Model/DriveWireHost.swift swift/DriveWireTests/DriveWireTests.swift
git commit -m "Implement guest-to-host serial writes (SERWRITE/SERWRITEM/FASTWRITE)"
```

---

### Task 3: Host→guest reads (writeToChannel / SERREAD poll / SERREADM)

**Files:**
- Modify: `swift/DriveWire/Model/DriveWireHost.swift` (stubs `OP_SERREAD`, `OP_SERREADM`; new public API)
- Test: `swift/DriveWireTests/DriveWireTests.swift`

**Interfaces:**
- Consumes: `VirtualChannel.inputQueue`, `vserialChannel(_:)` (Task 1)
- Produces: `public func writeToChannel(_ data: Data, channel: UInt8)` — queues host-side data for the guest (Task 4's bridge calls this)
- Produces: `public func closeChannel(_ channel: UInt8)` — host-side close; guest learns via next poll status byte

- [ ] **Step 1: Failing tests**

```swift
    func testSerialReadNothingWaiting() throws {
        var s = Data([host!.OPSERREAD])
        host!.send(data: &s)
        XCTAssertEqual(read(bytes: 2), Data([0, 0]))
    }

    func testSerialReadSingleByte() throws {
        var open = Data([host!.OPSERINIT, 1])
        host!.send(data: &open)
        host!.writeToChannel(Data([0x21]), channel: 1)
        var s = Data([host!.OPSERREAD])
        host!.send(data: &s)
        // Response byte 1 = channel + 1, byte 2 = the data byte.
        XCTAssertEqual(read(bytes: 2), Data([2, 0x21]))
    }

    func testSerialReadBulkThenReadM() throws {
        var open = Data([host!.OPSERINIT, 1])
        host!.send(data: &open)
        host!.writeToChannel(Data("mdir\r".utf8), channel: 1)
        var poll = Data([host!.OPSERREAD])
        host!.send(data: &poll)
        // Byte 1 = channel + 17, byte 2 = waiting count.
        XCTAssertEqual(read(bytes: 2), Data([18, 5]))
        var readm = Data([host!.OPSERREADM, 1, 5])
        host!.send(data: &readm)
        XCTAssertEqual(read(bytes: 5), Data("mdir\r".utf8))
        var poll2 = Data([host!.OPSERREAD])
        host!.send(data: &poll2)
        XCTAssertEqual(read(bytes: 2), Data([0, 0]))
    }

    func testSerialReadMOverRequestSendsNothing() throws {
        var open = Data([host!.OPSERINIT, 1])
        host!.send(data: &open)
        host!.writeToChannel(Data([0x01]), channel: 1)
        var readm = Data([host!.OPSERREADM, 1, 200])
        host!.send(data: &readm)
        XCTAssertEqual(responseData.count, 0)
    }

    func testHostCloseReportedInPoll() throws {
        var open = Data([host!.OPSERINIT, 4])
        host!.send(data: &open)
        host!.closeChannel(4)
        var poll = Data([host!.OPSERREAD])
        host!.send(data: &poll)
        // Byte 1 = 16 (status), byte 2 high nibble 0 = closed, low nibble = channel.
        XCTAssertEqual(read(bytes: 2), Data([16, 4]))
        XCTAssertFalse(host!.virtualChannels[4].isOpen)
    }

    func testSerialReadRoundRobinFairness() throws {
        var open = Data([host!.OPSERINIT, 1, host!.OPSERINIT, 2])
        host!.send(data: &open)
        host!.writeToChannel(Data([0x41]), channel: 1)
        host!.writeToChannel(Data([0x42]), channel: 2)
        var poll = Data([host!.OPSERREAD])
        host!.send(data: &poll)
        let first = read(bytes: 2)
        var poll2 = Data([host!.OPSERREAD])
        host!.send(data: &poll2)
        let second = read(bytes: 2)
        XCTAssertEqual(Set([first[0], second[0]]), Set([UInt8(2), UInt8(3)]))
    }
```

- [ ] **Step 2: Run, verify failure** (`writeToChannel` doesn't exist → compile error).

- [ ] **Step 3: Implement**

```swift
    /// Round-robin cursor so one busy channel can't starve the others.
    private var nextPollChannel = 0
    /// Host-side closes not yet reported to the guest via SERREAD.
    private var pendingClosedChannels : [UInt8] = []

    /// Queues data for the guest to read from a virtual channel.
    ///
    /// The guest collects queued bytes with its normal OP_SERREAD polling.
    /// Bytes queued while the channel is closed are discarded when the guest
    /// opens it (opening clears the queue), so callers should only write to
    /// channels the guest has opened.
    ///
    /// - Parameters:
    ///     - data: The bytes to queue.
    ///     - channel: The virtual serial channel number (0-14).
    public func writeToChannel(_ data : Data, channel : UInt8) {
        vserialChannel(channel)?.inputQueue.append(data)
    }

    /// Closes a channel from the host side.
    ///
    /// The guest is informed through a status byte in its next OP_SERREAD poll.
    ///
    /// - Parameters:
    ///     - channel: The virtual serial channel number (0-14).
    public func closeChannel(_ channel : UInt8) {
        guard let ch = vserialChannel(channel) else { return }
        ch.close()
        pendingClosedChannels.append(channel)
    }

    /// Builds the two-byte OP_SERREAD/POLL response described in the specification.
    private func serialReadResponse() -> Data {
        // Closed-channel notifications take priority over data.
        if pendingClosedChannels.isEmpty == false {
            let closed = pendingClosedChannels.removeFirst()
            return Data([16, closed])
        }
        // Round-robin scan for an open channel with waiting data.
        for offset in 0..<virtualChannels.count {
            let ch = virtualChannels[(nextPollChannel + offset) % virtualChannels.count]
            guard ch.isOpen, ch.inputQueue.isEmpty == false else { continue }
            nextPollChannel = (Int(ch.channelNumber) + 1) % virtualChannels.count
            if ch.inputQueue.count == 1 {
                // A single interactive byte rides along in the response itself.
                let byte = ch.inputQueue.removeFirst()
                return Data([ch.channelNumber + 1, byte])
            }
            // More is waiting: advertise the count for an OP_SERREADM fetch.
            return Data([ch.channelNumber + 17, UInt8(min(ch.inputQueue.count, 255))])
        }
        return Data([0, 0])
    }

    private func OP_SERREAD(data : Data) -> Int {
        currentTransaction = OPSERREAD
        resetState()
        delegate?.dataAvailable(host: self, data: serialReadResponse())
        delegate?.transactionCompleted(opCode: currentTransaction)
        return 1
    }

    private func OP_SERREADM(data : Data) -> Int {
        let expectedCount = 3
        guard data.count >= expectedCount else { return 0 }
        currentTransaction = OPSERREADM
        resetState()
        let requested = Int(data[2])
        if let ch = vserialChannel(data[1]), ch.inputQueue.count >= requested {
            let bytes = Data(ch.inputQueue.prefix(requested))
            ch.inputQueue.removeFirst(requested)
            delegate?.dataAvailable(host: self, data: bytes)
        } else {
            // Per the specification, over-asking is a guest error; the server
            // deliberately sends nothing and lets the guest's read time out.
            log = log + "OP_SERREADM over-request on channel \(data[1])" + "\n"
        }
        delegate?.transactionCompleted(opCode: currentTransaction)
        return expectedCount
    }
```

Replace the two stubs; put `writeToChannel`/`closeChannel`/`serialReadResponse` beside them; the two stored properties go next to `virtualChannels`.

- [ ] **Step 4: Run all serial tests (Tasks 1–3 names) — expect `** TEST SUCCEEDED **`.**

- [ ] **Step 5: Commit**

```bash
git add swift/DriveWire/Model/DriveWireHost.swift swift/DriveWireTests/DriveWireTests.swift
git commit -m "Implement host-to-guest serial reads (SERREAD poll/SERREADM) and host channel API"
```

---

### Task 4: DriveWireTCPServerDriver (becker listener + channel bridges)

**Files:**
- Create: `swift/DriveWire/Drivers/DriveWireTCPServerDriver.swift`
- Modify: `swift/DriveWire.xcodeproj/project.pbxproj` (all 3 targets, same recipe as Task 1 Step 5)
- Test: `swift/DriveWireTests/DriveWireTests.swift`

**Interfaces:**
- Consumes: `DriveWireHost.send(data:)`, `writeToChannel(_:channel:)`, `channelDataAvailable` (Tasks 1–3)
- Produces: `class DriveWireTCPServerDriver: NSObject, DriveWireDelegate { init(beckerPort: UInt16, channelPortBase: UInt16, bridgedChannelCount: Int); var host: DriveWireHost; var logging: Bool; func start() throws; func stop() }`

- [ ] **Step 1: Failing integration test** (loopback through real sockets; high ports to avoid collisions):

```swift
    func testTCPServerLoopback() throws {
        let driver = DriveWireTCPServerDriver(beckerPort: 62504, channelPortBase: 62810, bridgedChannelCount: 2)
        driver.logging = false
        try driver.start()

        let guest = NWConnection(host: "127.0.0.1", port: 62504, using: .tcp)
        let responded = XCTestExpectation(description: "OP_DWINIT answered over TCP")
        guest.stateUpdateHandler = { state in
            if case .ready = state {
                guest.send(content: Data([0x5A, 0x01]), completion: .contentProcessed { _ in })
                guest.receive(minimumIncompleteLength: 1, maximumLength: 16) { content, _, _, _ in
                    XCTAssertEqual(content?.first, 0x00)
                    responded.fulfill()
                }
            }
        }
        guest.start(queue: .global())
        wait(for: [responded], timeout: 5.0)
        guest.cancel()
        driver.stop()
    }
```

(`import Network` at the top of the test file.)

- [ ] **Step 2: Run, verify compile failure** (type doesn't exist).

- [ ] **Step 3: Create `swift/DriveWire/Drivers/DriveWireTCPServerDriver.swift`**

```swift
//
//  DriveWireTCPServerDriver.swift
//  DriveWire
//

import Foundation
import Network

/// Serves DriveWire to guests that connect over TCP, such as XRoar's becker port.
///
/// Unlike ``DriveWireTCPDriver``, which dials out to a guest, this driver
/// *listens*: the emulator connects to ``beckerPort`` (XRoar's default is
/// 65504). Each virtual serial channel N is also exposed as its own listening
/// TCP port (``channelPortBase`` + N); bytes flow between whatever connects
/// there and the guest's corresponding `/N` device.
///
/// All Network callbacks are delivered on the main queue: the host's internal
/// watchdog timer needs a running main run loop, which the CLI provides.
class DriveWireTCPServerDriver : NSObject, DriveWireDelegate, ObservableObject {
    /// The host object.
    internal var host = DriveWireHost()

    /// A flag that when set to `true` causes traffic to log.
    public var logging = false

    /// The TCP port the guest's emulator connects to.
    public let beckerPort : UInt16

    /// Virtual serial channel N is served on TCP port `channelPortBase + N`.
    public let channelPortBase : UInt16

    /// The number of channels that get TCP bridge ports.
    public let bridgedChannelCount : Int

    private var beckerListener : NWListener?
    private var guestConnection : NWConnection?
    private var channelListeners : [NWListener] = []
    private var channelClients : [UInt8 : NWConnection] = [:]
    /// Per-channel guest output that arrived while no client was attached.
    private var channelBacklog : [UInt8 : Data] = [:]
    private let backlogLimit = 65536

    /// Creates a listening DriveWire server.
    ///
    /// - Parameters:
    ///     - beckerPort: The TCP port to accept the emulator on.
    ///     - channelPortBase: Channel N is bridged on this port plus N.
    ///     - bridgedChannelCount: How many channels to bridge.
    init(beckerPort : UInt16 = 65504, channelPortBase : UInt16 = 6810, bridgedChannelCount : Int = 4) {
        self.beckerPort = beckerPort
        self.channelPortBase = channelPortBase
        self.bridgedChannelCount = bridgedChannelCount
        super.init()
        host = DriveWireHost(delegate: self)
    }

    /// Starts the becker listener and one bridge listener per channel.
    public func start() throws {
        beckerListener = try makeListener(port: beckerPort) { [weak self] connection in
            self?.acceptGuest(connection)
        }
        for n in 0..<bridgedChannelCount {
            let channel = UInt8(n)
            let listener = try makeListener(port: channelPortBase + UInt16(n)) { [weak self] connection in
                self?.acceptClient(connection, channel: channel)
            }
            channelListeners.append(listener)
        }
    }

    /// Stops all listeners and closes all connections.
    public func stop() {
        beckerListener?.cancel()
        guestConnection?.cancel()
        channelListeners.forEach { $0.cancel() }
        channelClients.values.forEach { $0.cancel() }
        channelListeners.removeAll()
        channelClients.removeAll()
    }

    private func makeListener(port : UInt16, accept : @escaping (NWConnection) -> Void) throws -> NWListener {
        let listener = try NWListener(using: .tcp, on: NWEndpoint.Port(rawValue: port)!)
        listener.newConnectionHandler = accept
        listener.start(queue: .main)
        return listener
    }

    private func acceptGuest(_ connection : NWConnection) {
        // A newly connecting emulator (e.g. after a reset) replaces the old one.
        guestConnection?.cancel()
        guestConnection = connection
        if logging { print("guest connected on becker port \(beckerPort)") }
        connection.start(queue: .main)
        receiveGuestBytes(connection)
    }

    private func receiveGuestBytes(_ connection : NWConnection) {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 1024) { [weak self] content, _, isComplete, error in
            guard let self = self else { return }
            if var data = content, data.isEmpty == false {
                if self.logging { data.dump(prefix: "->") }
                self.host.send(data: &data)
            }
            if isComplete || error != nil {
                if self.logging { print("guest disconnected") }
                if connection === self.guestConnection { self.guestConnection = nil }
                connection.cancel()
                return
            }
            self.receiveGuestBytes(connection)
        }
    }

    private func acceptClient(_ connection : NWConnection, channel : UInt8) {
        channelClients[channel]?.cancel()
        channelClients[channel] = connection
        if logging { print("client connected to channel \(channel)") }
        connection.start(queue: .main)
        if let backlog = channelBacklog.removeValue(forKey: channel), backlog.isEmpty == false {
            connection.send(content: backlog, completion: .contentProcessed { _ in })
        }
        receiveClientBytes(connection, channel: channel)
    }

    private func receiveClientBytes(_ connection : NWConnection, channel : UInt8) {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 1024) { [weak self] content, _, isComplete, error in
            guard let self = self else { return }
            if let data = content, data.isEmpty == false {
                self.host.writeToChannel(data, channel: channel)
            }
            if isComplete || error != nil {
                // A departing client does NOT close the guest's channel; the
                // next client picks the session up where it left off.
                if self.logging { print("client left channel \(channel)") }
                if connection === self.channelClients[channel] {
                    self.channelClients[channel] = nil
                }
                connection.cancel()
                return
            }
            self.receiveClientBytes(connection, channel: channel)
        }
    }

    @_documentation(visibility: private)
    internal func transactionCompleted(opCode : UInt8) {
    }

    @_documentation(visibility: private)
    internal func dataAvailable(host : DriveWireHost, data : Data) {
        if logging { data.dump(prefix: "<-") }
        guestConnection?.send(content: data, completion: .contentProcessed { _ in })
    }

    @_documentation(visibility: private)
    internal func channelDataAvailable(host : DriveWireHost, channel : UInt8, data : Data) {
        if let client = channelClients[channel] {
            client.send(content: data, completion: .contentProcessed { _ in })
        } else {
            var backlog = channelBacklog[channel, default: Data()]
            backlog.append(data)
            if backlog.count > backlogLimit {
                backlog.removeFirst(backlog.count - backlogLimit)
                if logging { print("channel \(channel) backlog trimmed") }
            }
            channelBacklog[channel] = backlog
        }
    }
}
```

- [ ] **Step 4: pbxproj entries** (3 targets, same recipe as Task 1 Step 5).

- [ ] **Step 5: Run `testTCPServerLoopback` — expect `** TEST SUCCEEDED **`.**
  Caveat: XCTest runs need a main run loop for `.main`-queue callbacks; `wait(for:timeout:)` spins one, so this works. If it flakes, switch the driver to an internal `DispatchQueue` parameter and pass `.main` only in the CLI — but try the simple form first.

- [ ] **Step 6: Commit**

```bash
git add swift/DriveWire/Drivers/DriveWireTCPServerDriver.swift swift/DriveWire.xcodeproj/project.pbxproj swift/DriveWireTests/DriveWireTests.swift
git commit -m "Add DriveWireTCPServerDriver: becker-port listener with per-channel TCP bridges"
```

---

### Task 5: drivewire-cli --becker mode

**Files:**
- Modify: `swift/drivewire-cli/main.swift`

**Interfaces:**
- Consumes: `DriveWireTCPServerDriver` (Task 4)
- Produces: CLI flags `--becker-port <UInt16>`, `--channel-port-base <UInt16>` (default 6810), `--channels <Int>` (default 4); `--port` becomes optional; exactly one of `--port`/`--becker-port` required.

- [ ] **Step 1: Rewrite `main.swift`'s option handling**

```swift
struct DriveWireCmd: ParsableCommand {
    @Option(name: .shortAndLong, help: "Serial port path (e.g. /dev/cu.usbserial-FTVA079L)")
    var port: String?

    @Option(name: .shortAndLong, help: "Baud rate for the serial port")
    var baudRate: Int = 57600

    @Option(name: .long, help: "Listen for a TCP guest (e.g. XRoar's becker port) on this port")
    var beckerPort: UInt16?

    @Option(name: .long, help: "Virtual serial channel N is bridged on TCP port base+N")
    var channelPortBase: UInt16 = 6810

    @Option(name: .long, help: "Number of virtual serial channels to bridge over TCP")
    var channels: Int = 4

    @Option(name: .long, help: "Virtual disk image path to insert into drive 0")
    var disk0: String?
    // ... disk1/disk2/disk3 unchanged ...

    @Flag(name: .shortAndLong, help: "Show client activity")
    var verbose: Bool = false

    func validate() throws {
        guard (port == nil) != (beckerPort == nil) else {
            throw ValidationError("Specify exactly one of --port (serial) or --becker-port (TCP listen).")
        }
    }

    func run() throws {
        let host: DriveWireHost
        var keepAlive: [AnyObject] = []

        if let port {
            let d = DriveWireSerialDriver()
            d.baudRate = baudRate
            d.portName = port
            d.logging = verbose
            host = d.host
            keepAlive.append(d)
        } else {
            let d = DriveWireTCPServerDriver(beckerPort: beckerPort!,
                                             channelPortBase: channelPortBase,
                                             bridgedChannelCount: channels)
            d.logging = verbose
            try d.start()
            host = d.host
            keepAlive.append(d)
            print("DriveWire listening: becker :\(beckerPort!), channels 0..<\(channels) on :\(channelPortBase)+N")
        }

        if let disk0Path = disk0 { try host.insertVirtualDisk(driveNumber: 0, imagePath: disk0Path) }
        if let disk1Path = disk1 { try host.insertVirtualDisk(driveNumber: 1, imagePath: disk1Path) }
        if let disk2Path = disk2 { try host.insertVirtualDisk(driveNumber: 2, imagePath: disk2Path) }
        if let disk3Path = disk3 { try host.insertVirtualDisk(driveNumber: 3, imagePath: disk3Path) }

        withExtendedLifetime(keepAlive) {
            while true {
                RunLoop.current.run(mode: .default, before: Date.distantFuture)
            }
        }
    }
}
```

(Keep the existing header comment and `DriveWireCmd.main()`; preserve disk1–3 options verbatim.)

- [ ] **Step 2: Build the CLI**

```bash
xcodebuild -project swift/DriveWire.xcodeproj -scheme drivewire-cli -configuration Debug build 2>&1 | tail -3
```
Expected: `** BUILD SUCCEEDED **`

- [ ] **Step 3: Smoke test by hand**

```bash
BUILT=$(xcodebuild -project swift/DriveWire.xcodeproj -scheme drivewire-cli -configuration Debug -showBuildSettings 2>/dev/null | awk '/ BUILT_PRODUCTS_DIR/{print $3; exit}')
"$BUILT/drivewire-cli" --becker-port 62504 --verbose &
sleep 1
printf '\x5a\x01' | nc -w 2 127.0.0.1 62504 | xxd
kill %1
```
Expected: one `00` byte back (OP_DWINIT response), verbose log shows the traffic.

- [ ] **Step 4: Commit**

```bash
git add swift/drivewire-cli/main.swift
git commit -m "drivewire-cli: add --becker-port TCP listen mode with channel bridges"
```

---

### Task 6: Guest disk prep (dw_becker variant + setime fix + /N1 shell)

**Files:**
- Create: `os9/nitros9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker/` (extracted from zip; NOT in any git repo)

**Interfaces:**
- Consumes: ToolShed `os9` binary at `os9/nitros9/tools/toolshed/` (already built)
- Produces: a bootable, unattended `68IDE.ide` with a shell listening on `/N1`, used by Tasks 7–8.

- [ ] **Step 1: Extract the dw_becker variant**

```bash
cd /Users/rdoggett/mine/os9/XXX/os9exec/os9/nitros9/disk-images
unzip eou_ide-6809-bin-v0.3.zip '6809_ide_xroar_dw_becker/*' -d eou-tmp
mv eou-tmp/6809_ide_xroar_dw_becker eou_ide-v0.3-6809-xroar-dw-becker
rmdir eou-tmp
cp eou_ide-v0.3-6809-xroar-dw-becker/68IDE.ide eou_ide-v0.3-6809-xroar-dw-becker/68IDE.ide.orig-backup
```

- [ ] **Step 2: Extract the RBF partition and sanity-check it** (recipe from the setime fix, memory `feature-xroar-autoboot-and-drivewire-fixes`):

```bash
cd eou_ide-v0.3-6809-xroar-dw-becker
dd if=68IDE.ide of=partition.img bs=512 skip=632
# LSN0: DD.TOT = bytes 0-2 big-endian, sector size 256 -- verify:
xxd -l 8 partition.img
python3 -c "import os; d=open('partition.img','rb').read(3); tot=int.from_bytes(d,'big'); print(tot*256, os.path.getsize('partition.img'))"
```
Expected: the two numbers match (as they did for the plain variant). **If they don't, STOP — the partition offset differs on this variant; re-derive it by scanning for the RBF LSN0 signature before writing anything back.**

- [ ] **Step 3: Edit `startup`** — dump it, apply BOTH changes (delete `setime<>>>/1` line; insert `shell <>>>/n1&` immediately BEFORE the `cd sys`/font-merge block, since nothing after that block executes reliably):

```bash
OS9=../..//tools/toolshed/os9   # adjust to the real built path
$OS9 copy -l partition.img,startup startup.txt
cp startup.txt startup.txt.orig
# edit startup.txt: remove the setime line; add the shell line before the font block
$OS9 del partition.img,startup
$OS9 copy -l startup.txt partition.img,startup
$OS9 copy -l partition.img,startup startup.verify && diff startup.txt startup.verify
```

- [ ] **Step 4: Write the partition back and snapshot the known-good state**

```bash
dd if=partition.img of=68IDE.ide bs=512 seek=632 conv=notrunc
cp 68IDE.ide 68IDE.ide.n1shell-backup
```

- [ ] **Step 5: Regression-check the boot** (before involving DriveWire at all):

```bash
xroar -rompath ../../roms -machine coco3 -tv-input rgb -machine-cart ide \
      -cart-rom ./hdblba.rom -load-hd0 68IDE.ide -cart-becker \
      -type 'DOS 0\r\r' -ui null -ao null -timeout 90
```
Expected: exits after timeout without crashing. (With `-ui null` we can't see the screen; the real boot verification is Task 7's channel traffic. If XRoar objects to `-cart-becker` with nothing listening, note it and start the server first in all future steps.)

No commit — this directory is disk-image working material, not in git.

---

### Task 7: Live bring-up (XRoar ↔ drivewire-cli ↔ nc)

**Files:** none created; this is the integration checkpoint.

**Interfaces:**
- Consumes: Task 5's CLI binary, Task 6's disk.
- Produces: the *facts* Task 8's script needs: (a) `/N1` ↔ wire-channel-number ↔ TCP-port mapping; (b) the shell prompt string on the channel; (c) boot-to-prompt latency.

- [ ] **Step 1: Start the server in tmux, then XRoar**

```bash
tmux new-session -d -s dwtest -x 200 -y 50
tmux send-keys -t dwtest "'$BUILT/drivewire-cli' --becker-port 65504 --verbose 2>&1 | tee /tmp/dw.log" Enter
tmux split-window -t dwtest
tmux send-keys -t dwtest "cd /Users/rdoggett/mine/os9/XXX/os9exec/os9/nitros9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker && xroar -rompath ../../roms -machine coco3 -tv-input rgb -machine-cart ide -cart-rom ./hdblba.rom -load-hd0 68IDE.ide -cart-becker -type 'DOS 0\\r\\r' -ui null -ao null" Enter
```

- [ ] **Step 2: Watch for the wire coming up.** In `/tmp/dw.log` expect within ~90 s: OP_DWINIT traffic, then `OP_SERSETSTAT(...,41)` (SS.Open = 0x29 = 41 decimal) or `OP_SERINIT` for the channel `/N1` maps to, then FASTWRITE bytes (the shell's prompt). **Note which channel number appears — that fixes the /N1↔port mapping** (channel 0 → port 6810, channel 1 → 6811, …).

- [ ] **Step 3: Connect and drive it**

```bash
nc 127.0.0.1 <port from step 2>
```
Type `mdir` + Enter (NB: OS-9 wants CR; nc sends LF — if the shell doesn't respond, retry with `stty -icrnl`/`socat` CR translation, and note what worked). Expected: a module directory listing. Record the exact prompt text.

- [ ] **Step 4: If nothing arrives at all**: fall back per the spec's debugging ladder — check `dw.log` for SERREAD polls (proves scdwv is alive); check the startup edit actually took (re-dump `startup` via ToolShed); try `tsmon /n1&` instead of `shell <>>>/n1&` (re-run Task 6 steps 3–4). Timebox each hypothesis; document findings in the plan file as you go.

---

### Task 8: tools/nitros9repl.sh + end-to-end verification

**Files:**
- Create: `os9exec-git_code/tools/nitros9repl.sh` (mode 755)

**Interfaces:**
- Consumes: facts from Task 7 (port, prompt regex, latency), Task 5 CLI, Task 6 disk.
- Produces: `nitros9repl.sh start|send|key|snap|connect|stop|restart` — same contract as `os9repl.sh`.

- [ ] **Step 1: Write the script.** Skeleton (flesh out with Task 7's real values for `CHAN_PORT`, `PROMPT_RE`, boot latency):

```bash
#!/bin/bash
# nitros9repl.sh -- drive a live NitrOS-9 (6809, XRoar+DriveWire) shell from
# scripts, mirroring tools/os9repl.sh's interface. See that script for the
# general tmux conventions. The shell arrives over DriveWire virtual serial
# channel /N1, bridged to TCP by drivewire-cli --becker-port.
#
# Usage:
#   ./tools/nitros9repl.sh start          boot XRoar + drivewire-cli, wait for /N1 shell
#   ./tools/nitros9repl.sh send <cmd>     send one command, wait for prompt, print new output
#   ./tools/nitros9repl.sh key <keys...>  raw keystrokes (no Enter)
#   ./tools/nitros9repl.sh snap [label]   labeled snapshot of the channel pane
#   ./tools/nitros9repl.sh connect        interactive session for a human (Ctrl-C detaches;
#                                          the OS-9 session survives disconnects)
#   ./tools/nitros9repl.sh stop|restart
SESSION=nitros9repl
NITROS9=/Users/rdoggett/mine/os9/XXX/os9exec/os9/nitros9
DISKDIR=$NITROS9/disk-images/eou_ide-v0.3-6809-xroar-dw-becker
CLI=...        # built drivewire-cli path (resolve via xcodebuild -showBuildSettings or a fixed copy)
BECKER_PORT=65504
CHAN_PORT=6811                     # fixed by Task 7's observed mapping
PROMPT_RE='...'                    # fixed by Task 7's observed prompt
...
```

Structure it exactly like `os9repl.sh`: `pane()` captures the nc window, `at_prompt()` greps `PROMPT_RE`, `wait_prompt()` polls, `cmd_send` uses `tmux send-keys` into the nc pane. `cmd_start` launches three tmux windows (cli, xroar, `nc`), waits for the prompt. `cmd_connect` executes `nc 127.0.0.1 $CHAN_PORT` in the caller's own terminal (foreground). CR handling: whatever Task 7 established (likely pipe through `tr`/`socat`).

- [ ] **Step 2: End-to-end test**

```bash
./tools/nitros9repl.sh start        # expect "[ready]" + prompt within the observed latency
./tools/nitros9repl.sh send mdir    # expect module listing incl. scdwv
./tools/nitros9repl.sh send dir     # expect directory listing
./tools/nitros9repl.sh stop
```

- [ ] **Step 3: Re-run the full Swift serial test suite one last time** (all Task 1–4 test names) — expect `** TEST SUCCEEDED **`.

- [ ] **Step 4: Commit (this repo)**

```bash
git add tools/nitros9repl.sh
git commit -m "Tools: add nitros9repl.sh, scripted REPL for NitrOS-9 via DriveWire /N1"
```

---

### Task 9: Documentation + memory + PR draft (not submitted)

**Files:**
- Create: `docs/superpowers/plans/…` checkboxes updated as executed
- Create: memory files (auto-memory directory) — REPL usage + drivewire status
- Create: `/tmp`-scratchpad PR draft text for user review

- [ ] **Step 1: Update auto-memory**: new memory `feature-drivewire-virtual-serial-repl` (what shipped, port mapping, prompt, gotchas found live); update `feature-xroar-autoboot-and-drivewire-fixes` (its "not yet working" /N1 section is now superseded); update MEMORY.md index lines.
- [ ] **Step 2: Update the os9-dev skill**: add a short "6809 live verification via nitros9repl.sh" pointer next to the existing 68k REPL reference (`references/common/using-os9exec-repl.md` sibling note) — content only for live-verified facts.
- [ ] **Step 3: Draft the upstream PR description** (summary of engine + driver + CLI, spec citations, test list) and leave it in the scratchpad for the user; do NOT push or open anything.
- [ ] **Step 4: Final `git log --oneline` of both repos in the session summary for the user.**

---

## Self-review notes

- Spec coverage: lifecycle (T1), guest→host (T2), host→guest (T3) — all nine opcodes; becker listener + bridges (T4); CLI (T5); disk (T6); live facts (T7); REPL (T8); docs/PR-draft (T9). VWindow explicitly consumed-and-logged (T2 FASTWRITE_Screen, T1 vserialChannel nil-guard).
- Type consistency: `virtualChannels`, `writeToChannel(_:channel:)`, `closeChannel(_:)`, `channelDataAvailable(host:channel:data:)` used identically in T1–T5.
- Known judgment calls recorded: single-byte poll delivery only when exactly 1 byte waits; over-request → silence (spec-mandated); client disconnect ≠ channel close; open() clears stale queue.
