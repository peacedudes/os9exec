# OS9exec — arm64 port

OS9exec is an OS-9/68k emulator originally written by Lukas Zeller and
Beat Forster for Classic Mac OS and later ported to Windows and Linux.
This branch (`arm64-uae-integration`) ports it to **macOS on Apple Silicon**
(arm64), replacing the original 68k interpreter with the UAE 68000 engine
and fixing all 64-bit data-width bugs introduced by the LP64 memory model.

Original project: <http://www.synthesis.ch/os9exec>  
Source repository: <https://sourceforge.net/p/os9exec/git_code/ci/master/tree/>  
License: GNU General Public License v2 (see source file headers)


## Compatibility

General OS-9/68k programs run correctly. The emulator exercises the full
syscall surface — file I/O, process management, module loading, pipes,
events, signals, and the shell — and these all behave as expected.

What you have in your `CMDS` directory depends on your disk image; the
commands there are yours to discover with `-h`. People who have OS-9
software are not newbies.

### Known not working

| Item | Issue |
|------|-------|
| `screen` | Crashes in `F$CpyMem` (A0 = 0xFFFFFFF2); uses Mac QuickDraw globals at negative A5-relative offsets not present in the emulation |
| `setime` | Accepts only 2-digit years; goes into infinite loop on bad date format |
| `c68` | Reads from stdin when no file args given — hangs in non-interactive use |
| `break` / interactive debugger | Terminal is left in raw mode after exit; shell becomes unresponsive |
| Raw device commands | `backup`, `format`, `rdump`, `frestore`/`fsave`, `tape`, `com`, `kermit` — require physical hardware or raw device access not emulated |

### Shell features

`>` stdout redirect · `>>` stderr redirect (not append — OS-9 convention) ·
`<` stdin redirect · `!` pipe · `;` sequential · `&` concurrent ·
`/pipe/name` named pipes (disappear when drained)


## Requirements

- macOS 14+ on Apple Silicon (arm64)
- Xcode Command Line Tools (`xcode-select --install`)
- Something for `/dd`: an RBF disk image, or simply a host directory
  containing OS-9 binaries. A minimal setup is one directory with one
  OS-9 executable module — no full OS-9 installation required.
  Point `OS9DISK` at it, or place/symlink it as `dd` in the repo root.
- The four adapted OS-9 header files in `Source/OS9exec_core/os9defs/`
  (see below — these are not included due to copyright)


## Build

**Requirements:** Xcode Command Line Tools (`xcode-select --install`) —
provides Apple clang and GNU make. No other tools are needed for the build
itself. (`gtimeout`, from Homebrew `coreutils`, is used by the test suite
but not the build.)

```sh
make
```

Produces `os9exec` in the repo root. The build uses Apple clang with the
UAE 68000 engine; `-m32` is not used.


## Quickstart — using the Microware OS-9/68k SDK

The commands in the `CMDS catalog` section below come from the Microware
OS-9/68k SDK (the 68000 CMDS directory found on network and developer
distributions).  If you have a licensed copy, here is how to set up a
working `dd/` directory:

```sh
# 1. Create the disk directory next to the os9exec binary
mkdir -p dd/CMDS

# 2. Copy the OS-9 utilities from your SDK
#    Adjust the path to match where your SDK is mounted or unpacked.
#    A common layout is OS9/68000/CMDS inside the disk image or archive.
cp /path/to/sdk/OS9/68000/CMDS/* dd/CMDS/

# 3. Create a minimal startup script (optional but recommended)
#    OS-9 uses carriage return (0x0D) as its line ending.
printf '* OS-9 startup\r\nchd /dd\r\nshell\r\n' > dd/startup

# 4. Run the shell
OS9DISK=$(pwd)/dd ./os9exec /dd/CMDS/shell
```

You can also symlink your SDK's CMDS directory instead of copying:

```sh
ln -s /path/to/sdk/OS9/68000/CMDS dd/CMDS
```

The `dd/` directory (or any path pointed to by `OS9DISK`) does not need to
be an RBF disk image — a plain host directory works fine for running
utilities.  RBF images are only required for commands that manipulate the
disk structure itself (`dcheck`, `format`, `free` reporting disk usage,
etc.).


## Run

```sh
OS9DISK=/path/to/your/disk.image ./os9exec /dd/CMDS/shell
```

`OS9DISK` is the path to an RBF-format OS-9 disk image. It appears inside
the emulator as `/dd`. `OS9CMDS` and `OS9MDIR` can override the default
command and module search paths.

Send ESC (`\033`) on a line by itself to exit the shell cleanly.


## Devices and host filesystem

OS9exec maps two-character OS-9 device names to host paths at startup.

**`/dd`** — the default drive  
Resolved via the `OS9DISK` environment variable. If `OS9DISK` is not set,
os9exec looks for a file or directory named `dd` in the directory it was
launched from. If `dd` is an RBF image file it is mounted automatically;
if it is a directory, host files inside it are accessible directly.

**`/hX`** — host directory slots (X = any single character: `0`–`9`, `a`–`z`, …)  
Each `/hX` device maps to the env var `OS9HX`. For example:

```sh
export OS9H1=/Users/me/os9work
export OS9H2=/Volumes/OldDisk
./os9exec shell
# now /h1 and /h2 are live inside the emulator
```

If `OS9HX` is not set, os9exec looks for a directory named `hX` relative to
its launch directory (and one level up). `/h0` has one extra fallback: if
nothing is found it attempts to auto-mount the `dd` RBF image as `/h0`,
which accommodates OS-9 programs that address the default drive as `/h0`
rather than `/dd`. A symlink `h0 → dd` in the launch directory also works
and is what the original docs recommend.

Files and directories anywhere on the host filesystem can be reached through
any `/hX` slot — there is no restriction to the `dd` subtree.


## Useful options

```
os9exec [-options] <command> [args]
```

| Option | Effect |
|--------|--------|
| `-v` | Ctrl-C kills the emulator immediately (otherwise OS-9 processes intercept it) |
| `-i` | Disable built-in emulator commands (see below) |
| `-m n[k\|M]` | Give the first OS-9 process extra static storage |
| `-mm n[k\|M]` | Give **all** OS-9 processes extra static storage |
| `-p prio` | Run first process at priority `prio` (default 128) |
| `-x width` | Set MGR screen width (passed to OS-9 via `F$GProDsc`) |
| `-y height` | Set MGR screen height |
| `-t` | Enable timing measurements |
| `-d[n] msk` | Set debug info mask at level n (see `-dh` for mask values) |
| `-ih` | Print built-in command list and exit |
| `-h` | Print full option list and exit |

Options may use `-` or `/` as the prefix (e.g., `/v` is the same as `-v`).


## Built-in emulator commands

When `INT_CMD` is defined at build time (the default in this port), os9exec
intercepts a set of command names before the OS-9 shell can fail on them.
They run as C functions inside the emulator and are visible to the shell like
any other program:

| Command | What it does |
|---------|--------------|
| `ihelp` / `icmds` | List all built-in commands |
| `iprocs` | Show running OS-9 processes |
| `imdir` | Show loaded OS-9 modules |
| `ipaths` | Show open path list |
| `imem` | Show memory block list |
| `idevs` | Show mounted devices |
| `ihit` | Show directory cache hit rate |
| `idbg` / `debughalt` | Enter the emulator debug menu |
| `stop` / `shutdown` | Exit os9exec cleanly |
| `rename` | Rename a file or directory (host-filesystem aware) |
| `move` | Move files or directories |
| `ls` | Directory listing in extended format |
| `mount` / `unmount` | Mount or unmount an RBF image |
| `systime` | Emulation timing display |
| `iquit` | Set the quit flag |
| `icrash` | Access an invalid address (crash test) |

Pass `-i` to disable all of these and use only real OS-9 binaries.


## CMDS catalog

See [CMDS.md](CMDS.md) for the full command list with descriptions and status.

## Emulator debugger

Run `idbg` from the OS-9 shell (or set a stop mask — see below) to enter the
emulator's interactive debugger.  The prompt looks like:

```
# Pid=3: dbgmsk=$0001,$0000,$0000 stop=$0000 trigger='' (type ?<Enter> for hlp)
```

Commands are single letters, case-insensitive, optionally followed by an
address or process ID. The address is always a **68k virtual address** in hex
(no `$` prefix needed, e.g. `i 1000`).

### Inspection commands

| Command | What it does |
|---------|--------------|
| `r` | Show 68k registers (D0–D7, A0–A7, PC, SR) for the current process |
| `r xx` | Same, for process ID `xx` |
| `i` | Disassemble 10 instructions starting at PC |
| `i xxx` | Disassemble 10 instructions from address `xxx` |
| `l` | Hex dump 64 bytes (8 lines) starting at A7 (top of stack) |
| `l xxx` | Hex dump 64 bytes from address `xxx` |
| `.` | Continue the previous `i` or `l` — shows the next block |
| `p` | List all OS-9 processes (PID, state, name, module address) |
| `v` | Show memory map for the current process |
| `v xx` | Memory map for process `xx` |
| `m` | List all loaded OS-9 modules |
| `f` | List open paths (files, devices, pipes) for all processes |
| `f xx` | Open paths for process `xx` only |

### Flow control

| Command | What it does |
|---------|--------------|
| `x` | Resume — return to the OS-9 shell and continue running |
| `t` | Single-step one 68k instruction, then re-enter the debugger with a full register dump |
| `k xx` | Kill process `xx` (send it a fatal signal) |
| `q` | Quit the entire emulator immediately |

`t` resets itself after each step — type it again to step the next instruction.  The
full register dump shows D0–D7, A0–A7, USP/ISP, SR flags, and FP registers.

**For reliable single-stepping of one program:** enter the debugger via `break`
rather than `idbg`.  `break` prints "WARNING: Timesharing HALTED" and suspends the
scheduler — after that, `t` steps only your process and cannot jump to another.
Entering via `idbg` leaves timesharing running, so steps may execute any ready process.

### Diagnostic logging — debug mask

The emulator can print detailed trace output for specific subsystems as OS-9
programs run.  The mask is a hex bit-field; combine values with `+`.

```
d xx        set the normal-level debug mask to xx (e.g. d 6 → syscalls + file manager)
d 1, xx     set detail-level mask
d 2, xx     set deep-level mask
dh          print the full list of mask bits
```

Common mask bits (from `dh`):

| Value | What it traces |
|-------|---------------|
| `0001` | Anomalies: unhandled exceptions and unimplemented syscalls (on by default) |
| `0002` | Every syscall entry and return status |
| `0004` | File manager type selection |
| `0008` | Process creation and death |
| `0010` | Task switching |
| `0020` | Module link / load / unlink |
| `0040` | Memory allocation and deallocation |
| `0200` | File open/close/read/write operations |
| `0400` | Error generation and translation |
| `1000` | Warnings for partially-emulated functions |
| `8000` | General warnings |

Example — trace syscalls and file operations:

```
d 202
```

To turn off all logging: `d 0`

### Stop mask — automatic debugger entry

The stop mask tells the emulator to drop into the debugger automatically when
a matching condition occurs, without needing to run `idbg` manually.

```
s x     set the stop mask to x (same bit values as the debug mask)
```

The most useful value is `s 1` — breaks into the debugger on any anomaly
(bus error, address error, unimplemented syscall).  This is the quickest way
to catch a crash at the point where it happens rather than discovering it
after the fact.

Turn off automatic breaks with `s 0`.


## Interactive REPL (tmux helper)

`tools/os9repl.sh` wraps os9exec in a detached tmux session and lets you (or
an automated tool such as Claude) drive it interactively — one command at a
time — without losing the shell or the debugger between calls.

**Requirements:** `tmux` (install via Homebrew: `brew install tmux`).

```sh
# First time: build, then start the session
make
./tools/os9repl.sh start

# Send shell commands; only the new output is printed
./tools/os9repl.sh send "dir /dd"
./tools/os9repl.sh send "echo hello"

# Enter the debugger and inspect process state
./tools/os9repl.sh send "idbg"
./tools/os9repl.sh send "p"          # process table
./tools/os9repl.sh send "v"          # memory map
./tools/os9repl.sh send "i 1000"     # disassemble from address $1000
./tools/os9repl.sh send "q"          # quit debugger

# See the full scrollback (useful after a crash)
./tools/os9repl.sh peek

# Rebuild and restart in one step
./tools/os9repl.sh restart

# Stop the session
./tools/os9repl.sh stop
```

Both the OS-9 shell prompt (`$`) and the emulator debugger prompt (`for hlp)`)
are recognised, so `send` works without modification whether you are at the
shell or inside `idbg`/`debughalt`.

**For Claude instances:** invoke `tools/os9repl.sh` via `Bash` tool calls
instead of running os9exec directly. The session persists across tool calls so
context is not lost between commands.

### Copyright notice — disk image content

OS-9 software (binaries, libraries, shell scripts) stored in your disk image
may be copyright Microware Systems Corporation or other parties. Everything
observable through the REPL — directory listings, file contents, program
output — reflects what is in your image. Do not use this tool to extract,
reproduce, or redistribute copyrighted content without appropriate
authorisation. The authors of os9exec make no representation about the
licensing status of any content you supply as a disk image.


## Test

Requires Swift (ships with Xcode):

```sh
make test
# or
swift run --package-path test
```

Run a subset by name:

```sh
swift run --package-path test -- echo
```


## OS-9 header files (copyright — not included)

The four files in `Source/OS9exec_core/os9defs/` must be supplied from a
licensed OS-9 system (V2.4, V2.9, or V3.0) and adapted before building.
See `Source/OS9exec_core/os9defs/defs_files.txt` for the full list of
required changes, including the arm64-specific type substitutions
(`long` → `uint32_t`, pointers → `os9addr_t`).


## arm64 port — what changed

The port involved three categories of work:

**1. UAE 68k engine integration**  
Wired in `Source/OS9AppEmu/UAE68emulator/` as the CPU interpreter,
replacing the original Mac-only 68k trap mechanism.

**2. 64-bit data-width fixes**  
On arm64, `unsigned long` is 64 bits. Every place the original code used
`ulong*` or `(ulong*)` to read or write a 32-bit field in the 68k arena
was writing 8 bytes instead of 4, silently corrupting memory. Fixed
throughout: `modstuff.c`, `fcalls.c`, `fileaccess.c`, `file_rbf.c`,
`pipefiles.c`, `os9exec_nt.c`, and the `mdir_entry` struct layout.

**3. OS-9 struct pointer fields**  
The adapted header files use `os9addr_t` (a `uint32_t` typedef) for all
fields that hold 68k virtual addresses. `TO68K()` / `FROM68K()` macros
in `os9_ll.h` translate between 68k arena offsets and host pointers.

**4. I$Create open-or-create fix**  
OS-9's `I$Create` syscall has open-or-create semantics: calling it on an
existing file should open it, not return an error. The original code
returned `E_CEF` (Creating Existing File) in this case. Fixed in
`fileaccess.c::pFopen`.


## Credits

Original OS9exec authors: Lukas Zeller, Beat Forster  
arm64 port: Robert Doggett, with Claude Sonnet 4.6 (Anthropic)
