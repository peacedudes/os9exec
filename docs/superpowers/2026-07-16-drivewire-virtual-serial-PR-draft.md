# DRAFT PR — drpitre/drivewire: implement virtual serial channels + TCP listen mode

Status: DRAFT for rdoggett's review. NOT submitted. Branch `virtual-serial`
in the local clone (`os9/nitros9/drivewire/drpitre-drivewire`), 8 commits,
nothing pushed.

---

**Title:** Implement virtual serial channels and a TCP listen mode for emulator guests

**Body:**

This implements the virtual serial channel side of the DriveWire protocol
in the Swift host, and adds a TCP *listening* driver so emulators like
XRoar and MAME (whose "becker port" connects out to a DriveWire server)
can use it. Everything follows `DriveWire Specification.md` in this repo.

## What's included

- **`VirtualChannel` + all nine serial opcodes in `DriveWireHost`**
  (`OP_SERINIT/SERTERM/SERSETSTAT/SERGETSTAT/SERREAD/SERREADM/SERWRITE/
  SERWRITEM/FASTWRITE`), replacing the previous stubs. Each processor
  consumes the exact operand length from the spec so the byte stream can't
  desync (the stubs returned 1 unconditionally, which desynced on any real
  guest traffic). The SERREAD poll response implements the full response
  table: single interactive byte, bulk count for SERREADM, channel-closed
  status, with round-robin fairness between channels.
- **A new delegate callback** `channelDataAvailable(host:channel:data:)`
  with a protocol-extension default, so existing conformers compile
  unchanged.
- **`DriveWireTCPServerDriver`** (Network.framework): listens for the
  emulator on a becker port (default 65504) and bridges each virtual
  channel N to its own TCP port (`channelPortBase + N`) — connect with
  nc/telnet and you're talking to `/N1` on the guest. Output while no
  client is attached is buffered (bounded) and replayed on connect.
- **`drivewire-cli --becker-port`** listen mode alongside the existing
  serial mode (`--channel-port-base`, `--channels`; `validate()` requires
  exactly one of `--port`/`--becker-port`).
- **17 unit tests** covering channel lifecycle, writes (including opcodes
  split across reads), poll/read cycles, over-request handling, and a
  real-socket loopback test of the TCP driver.

## Two guest-compatibility fixes found by live NitrOS-9 testing

Tested end-to-end against NitrOS-9 EOU (dw_becker) under XRoar:

1. **OP_DWINIT must answer non-zero.** The NitrOS-9 driver interprets a
   zero response as a DW3 server and disables its DW4 extensions —
   including the virtual channel poller, so SERREAD polling never starts.
   The host now answers `0xFF`.
2. **The EOU boot emits bare `$64 $00` pairs** (OP_SERWRITEM naming a
   never-opened channel, with no count byte). Parsing a count there
   swallows the following ~100 bytes (including the guest's SERINIT).
   The host now consumes exactly two bytes for a SERWRITEM addressed to an
   unopened channel, matching how existing servers behave.

## Also fixed en route

- `DriveWireTests` target deployment 13.0 → 14.1 (`@Observable` in
  `DriveWireHost` requires 14+; the test target didn't build).
- `testInsert` referenced a nonexistent `DriveWireError` enum
  (→ `DriveWireHostError`).

## Verification

- `xcodebuild -scheme DriveWire test` — all new tests pass (two
  pre-existing tests reference a local disk image path and are untouched).
- Live: NitrOS-9 EOU boots under XRoar with `-cart-becker`; a shell on
  `/N1` is fully usable over `nc localhost 6811` — `mdir`, `dir`, `date`
  all round-trip.

---

## Commits on the branch (for the PR)

1. Project: fix DriveWireTests deployment target (14.1) and add VirtualChannel.swift
2. Implement virtual serial channel lifecycle (SERINIT/SERTERM/SERSETSTAT/SERGETSTAT)
3. Implement guest-to-host serial writes (SERWRITE/SERWRITEM/FASTWRITE)
4. Implement host-to-guest serial reads (SERREAD poll/SERREADM) and host channel API
5. Add DriveWireTCPServerDriver: becker-port listener with per-channel TCP bridges
6. drivewire-cli: add --becker-port TCP listen mode with channel bridges
7. Fix DWINIT capability byte and SERWRITEM boot-noise handling for NitrOS-9 guests
8. Align loopback test with the non-zero DWINIT capability response

(The `DriveWireHostError` test fix rode along in commit 2.)

## Before submitting (checklist for the user)

- [ ] rdoggett reviews this text and the diff (`git -C .../drpitre-drivewire log -p main..virtual-serial`)
- [ ] Decide whether to squash the two DWINIT-related commits into the earlier ones
- [ ] Fork drpitre/drivewire on GitHub, push branch, open PR (public action — user's call)
