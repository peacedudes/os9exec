# DriveWire virtual serial → NitrOS-9 REPL — design

Date: 2026-07-16
Status: approved (user: "just go")

## Goal

Give future Claude sessions a fully scripted REPL against a live NitrOS-9
(6809) shell — the 6809 equivalent of `tools/os9repl.sh` — with zero human
intervention, plus a `connect` mode so a human can attach interactively to
the same channel.

**Success criterion:** `tools/nitros9repl.sh start` followed by
`tools/nitros9repl.sh send mdir` returns real NitrOS-9 output, unattended.

## Background

- XRoar emulates a CoCo3 running NitrOS-9 EOU. Its "becker port" tunnels
  DriveWire protocol over TCP: XRoar connects *out* to a DriveWire server
  at `127.0.0.1:65504` (defaults; `-cart-becker` enables the port on the
  IDE cartridge — confirmed present in the installed XRoar's options).
- DriveWire's protocol includes *virtual serial channels* (`/N1`… devices
  on the guest) — exactly the ssh-like text channel wanted here.
- DrPitre's canonical server (`drpitre/drivewire`, cloned at
  `os9/nitros9/drivewire/drpitre-drivewire/`) has a Swift host with a
  working disk layer, but **every `OP_SER*` handler is a stub**
  (`OP_SERREAD` always answers "no data"; `OP_SERWRITE` doesn't consume
  its operands — a real guest write desyncs the stream). Its TCP driver is
  an incomplete outbound client; becker needs a listener. The full wire
  spec lives in the same repo (`DriveWire Specification.md`).
- The EOU zip's `6809_ide_xroar_dw_becker/` variant bakes `dwio_becker.sb`,
  `scdwv.dr`, `n_scdwv.dd`, `n1..n4_scdwv.dd`, `clock2_dw` into OS9Boot —
  `/N1../N4` are resident at boot. This sidesteps the unresolved
  `load n1dwv.dd` hang from the 2026-07-16 session entirely. Its launch
  script is identical to the plain variant's; only the disk differs.

## Architecture

```
tools/nitros9repl.sh (tmux + nc)      human: nitros9repl.sh connect
        │ TCP :6810+N  (channel bridge)
        ▼
drivewire-cli --becker        [drpitre/drivewire fork, branch virtual-serial]
  ├─ DriveWireTCPServerDriver (new) — NWListener :65504 + :6810+N per channel
  └─ DriveWireHost + VirtualChannel engine (new — replaces the OP_SER* stubs)
        ▲ TCP :65504  (XRoar's becker port connects out)
XRoar — coco3, -cart-becker, dw_becker 68IDE.ide
  dwio_becker + scdwv + /N1../N4 baked into OS9Boot
  startup puts a shell listener on /N1
```

## Components

### 1. Guest disk prep (host-side only; ToolShed + dd)

- Extract `6809_ide_xroar_dw_becker/` from
  `os9/nitros9/disk-images/eou_ide-6809-bin-v0.3.zip` alongside the
  existing variant directory.
- Apply the proven setime-fix recipe: `dd` the RBF partition out at
  512-byte sector 632, delete the `setime<>>>/1` line from `startup` via
  ToolShed (`os9 copy -l`), `dd` back with `conv=notrunc`. Backups before
  every in-place write (`.orig-backup`, and a known-good post-fix copy).
- Add the `/N1` listener line to `startup` **before** the font-merge block
  (the 2026-07-16 session proved nothing after that block runs). Exact
  incantation (`tsmon /n1&` vs a bare shell redirect) decided live.
- XRoar launch = the variant's own `start-ide.sh` command + `-rompath` +
  `-cart-becker` (+ headless flags `-ui null -ao null` where useful).

### 2. VirtualChannel engine in `DriveWireHost` (the upstream-valuable core)

- New `VirtualChannel` type: channel number, open/closed state, bounded
  host→guest input queue; guest→host output is delivered immediately via a
  new delegate callback `channelDataAvailable(host:channel:data:)`, added
  to `DriveWireDelegate` with a default no-op so existing drivers compile
  unchanged.
- Implement all nine serial opcodes per the in-repo spec, with exact
  operand lengths so the byte stream can never desync:
  - `OP_SERINIT ($45)` / `OP_SERTERM ($C5)` — 2 bytes; open/close channel.
  - `OP_SERSETSTAT ($C4)` — 3 bytes, except code `SS.ComSt ($28)` = 29
    bytes total; `SS.Open ($29)` opens, `SS.Close ($2A)` closes.
  - `OP_SERGETSTAT ($44)` — 3 bytes, logging only (per spec).
  - `OP_SERREAD ($43)` — 1 byte; respond with the 2-byte poll response:
    byte1 0 = nothing; 1–15 = single interactive byte for channel
    (byte1−1) in byte2; 16 = status byte (channel-closed reporting);
    17–31 = "(byte1−17) channel has byte2 bytes waiting, fetch via
    SERREADM". Multi-channel arbitration: simple fairness (rotate among
    channels with waiting data) — the spec's wait-counter scheme noted as
    a possible refinement, not required.
  - `OP_SERREADM ($63)` — 3 bytes; reply with exactly the requested count
    from the channel queue (never more than previously advertised).
  - `OP_SERWRITE ($C3)` — 3 bytes; `OP_SERWRITEM ($64)` — 3+count bytes;
    `OP_FASTWRITE ($80+ch)` for ch 0–14 — 2 bytes.
- Partial packets: reuse the existing `serialBuffer`/`processor` framing
  (return 0 = wait for more bytes).
- Channel state is runtime-only — deliberately **not** added to the
  class's `Codable` keys, so the GUI document format is untouched.
- VWindow channels (128–142) are out of scope; log and consume correctly.

### 3. `DriveWireTCPServerDriver` (new, `Drivers/`)

- Network.framework `NWListener` on the becker port (default 65504).
  Accepted guest bytes → `host.send()`; host `dataAvailable` → becker
  socket. Guest reconnect (XRoar reset) = accept replacement connection.
- One `NWListener` per channel at `channelPortBase + N` (base default
  6810). Client bytes → channel input queue; `channelDataAvailable` →
  client socket. One client per channel; client disconnect does NOT close
  the OS-9 channel (output buffered, bounded, oldest dropped with a log
  line).
- All host access serialized on one `DispatchQueue` — matches the repo's
  existing class+queue style; no actor rewrite of DrPitre's codebase.

### 4. `drivewire-cli`

- `--port` (serial) becomes one of two modes; add `--becker-port`
  (listen mode) and `--channel-port-base`. Exactly one of serial/becker
  required; `--diskN` / `--verbose` work in both modes.

### 5. `tools/nitros9repl.sh` (this repo)

- Mirrors `os9repl.sh`'s interface: `start`, `send`, `key`, `snap`,
  `stop`, `restart` — plus `connect` (interactive passthrough for a
  human). tmux session runs drivewire-cli, XRoar, and an `nc` pane to the
  channel port; `send` is prompt-gated like the 68k version.
- Session name must not collide with `os9repl.sh`'s (known concurrent-
  instance collision problem — use a distinct name, e.g. `nitros9repl`).

### 6. Testing

- **Unit** (`DriveWireTests`, `xcodebuild test`, no emulator): synthetic
  opcode streams — open/write/poll/read/close cycles; FASTWRITE; opcodes
  split across `send()` boundaries; interleaved disk+serial traffic;
  writes to closed channels; SERREADM over-request.
- **Live**: boot the dw_becker disk against `drivewire-cli --becker -v`;
  confirm OP_DWINIT and poll traffic in the log; `nc` to the channel
  port; get a shell; run `mdir`/`dir`. Then the same end-to-end through
  `nitros9repl.sh`.

## Upstream plan

Branch `virtual-serial` on the local clone; repo-style commits. PR text
drafted for the user's review before anything public happens (public
actions still gated — same standing as the pyDriveWire fixes).

## Risks / open items

- `/N1` ↔ wire channel number mapping (0- vs 1-based) determined live;
  the `--channel-port-base` flag absorbs either outcome.
- scdwv's real-world polling (spec documents 50–700 ms adaptive SERREAD)
  is the ultimate arbiter of correctness; verbose logs will show it.
- The dw_becker variant's `startup` may differ from the plain variant's;
  the setime line and insertion point are re-verified on that disk, not
  assumed.
- If the guest-side listener incantation misbehaves (`tsmon` vs shell
  redirect), fall back to deciding it interactively on the console first,
  then baking the winner into `startup`.
