# OS9exec — Complete OS-9/68k emulator

Run actual OS-9 binaries on your modern computer. OS9exec emulates the 68k processor and OS-9 kernel, giving you a real OS-9 shell with pipes, redirection, job control, and the complete filesystem.

**Platforms:** macOS (arm64/Intel), Linux (64-bit/32-bit), Windows (native, via mingw-w64), Docker.

---

## Quick start

```sh
git clone https://github.com/peacedudes/os9exec.git
cd os9exec-git_code
make
OS9DISK=/path/to/your/os9 ./os9exec /path/to/your/os9/CMDS/shell
```

That's it: **1)** build it, **2)** point `OS9DISK` at a directory (or disk image) with a `CMDS` folder full of OS-9 binaries, **3)** run.

```
$ dir /dd/CMDS
$ echo hello
$ procs
$ exit
```

Don't have OS-9 software yet? [Legitimate sources exist for hobbyists](https://www.icdia.co.uk/) — OS9exec doesn't include any (not ours to distribute).

Prefer a prebuilt binary, Docker, or Apple Container instead of building? See [Other ways to run it](#other-ways-to-run-it) below.

---

## What's new

- **`mount -k=<size>`** creates a ready-to-use blank OS-9 disk in one command — no separate `format` step. `mount -k=0` creates a plain host folder instead. See [Creating new disk images](#creating-new-disk-images).
- **`mount -r=<size>`** (RAM disks) now actually works — it was silently broken in every prior build.
- **OS-9's own `debug` command works for the first time** — full register display, single-stepping, breakpoints. See [OS-9 `debug` now works](#os-9-debug-now-works).

---

## macOS security note

First run may be blocked. Fix with:
```sh
xattr -d com.apple.quarantine os9exec
```
Or: System Settings → Privacy & Security → Allow

---

## The disk layout

OS-9 sees your files through a device called `/dd` (the "default drive"). Two ways to set it up — mix them freely:

**Native host directories** — the simplest approach. Make a directory tree and point `OS9DISK` at it. OS-9 programs see host files as OS-9 files. This is also the easiest way to move files between OS-9 and the host: drop something in a host subdirectory and it appears instantly inside the emulator, and vice versa.

```
dd/
  CMDS/       ← OS-9 executables
  SYS/        ← startup, password, etc.  (optional)
  DEFS/       ← header files             (optional)
  startup     ← auto-executed at boot    (optional)
```

**RBF disk images** — if you have an actual OS-9 disk image (from a real machine, a tape backup, or an SDK archive), point `OS9DISK` directly at the image file. The RBF file system is fully supported: `os9exec` mounts it read/write at `/dd`, just as real OS-9 hardware would.

You can also attach extra host directories as `/h1`, `/h2`, etc. (see [Devices](#devices-and-host-filesystem) below) — useful for bridging an RBF image to the host: mount the image at `/dd` and a host staging folder at `/h1`, then `copy /dd/myfile /h1/myfile` to extract.

---

## Devices and host filesystem

| Device | How to set | What it maps to |
|--------|------------|-----------------|
| `/dd`  | `OS9DISK=…` env var, or a `dd` file/dir next to the binary | Default drive — RBF image or host directory |
| `/h0`–`/h9`, `/ha`–`/hz` | `OS9H0=…` through `OS9HZ=…`, or files/dirs named `h0`–`hz` next to the binary | RBF disk images or host directories |

Files placed in a host directory appear immediately inside the emulator as OS-9 files, with no conversion needed for binary modules. Text files need OS-9 line endings (CR, `0x0D`) rather than Unix LF — the emulator handles this transparently for `I$ReadLn`/`I$WritLn`, but raw byte copies preserve whatever endings are in the file.

RBF disk images pointed to by `/h0`–`/hz` are auto-mounted on first access — `dir /h0/CMDS` works directly with no need to touch `/h0` first or run `mount`. Use `mount <image> <devname>` to attach an image under a name of your choosing.

### Creating new disk images

`mount` can also create a brand-new device, instead of attaching an existing one:

```
mount -r=<size> [<name>]      create an in-memory RAM disk, <size> in kBytes
mount -k=<size> h0..hz        create a ready-to-use blank RBF image on disk
mount -k=0      h0..hz        create a plain host directory instead
```

`<size>` accepts a bare number (bytes) or a `k`/`M`/`G` suffix (×1024/×1024²/×1024³) — the same convention `os9exec`'s own `-m`/`-mm` command-line options use.

**RAM disk** (`-r=<size>`): fully formatted and usable immediately — no `format` needed. Lives only in memory; gone on `unmount` or emulator exit. `<name>` must be an absolute path (defaults to `/r0` if omitted).

```
mount -r=2000 /scratch    # 2000 kB RAM disk named /scratch
dir /scratch
unmount scratch           # releases the memory (bare name is fine here)
```

**Blank disk image** (`-k=<size>`): writes a fully formatted, ready-to-use RBF image straight to `<dir-holding-the-binary>/hX` — the same place the `/h0`–`/hz` auto-mount convention already looks, so the new device works immediately in the same session with no extra step. Rounds `<size>` up to a valid sector/track/cluster boundary automatically. Target must be `h0`–`hz` (never `dd`) and must not already exist.

```
mount -k=1M h7
dir /h7
```

To populate a fresh image with real content (and verify the copy), `dsave` works against it like any other device:

```
makdir /dd/USR/TESTER/doctest
echo hello from the mount -k example >/dd/USR/TESTER/doctest/hello.txt
chd /dd/USR/TESTER/doctest
dsave -ive /h7
```

> **One limit worth knowing:** both `mount -r` and `mount -k` allocate from the emulator's own 68k memory arena (32MB by default, `-M` to change it) — not host RAM. Requesting a size near or above that ceiling fails with "No more memory."

**Device-resolution order, if you're layering these:** for any `/hX` path, `os9exec` checks, in order: (1) the `OS9Hx` environment variable, if set; (2) a file/dir named `hX` next to the binary — what `mount -k` writes; (3) one directory level up from the binary (a legacy fallback). An explicit `mount <file> <name>` (or `mount -r=`) call takes priority over all three for as long as the process keeps running.

### Devices stay inside their root

Each device is a self-contained OS-9 volume. From inside the emulator you cannot climb out of a device into the host filesystem: `..` at a device root resolves to the root itself (like `/..` == `/` on Unix), and an absolute path that names no configured device is rejected rather than dropping you into a real host directory.

If you *want* OS-9 to reach a specific host location, expose it deliberately as its own device:

```
ln -s /any/host/dir h5      # now /h5 inside OS-9 is that directory
```

---

## Options

```
./os9exec [-options] <command> [args]
```

| Option | Effect |
|--------|--------|
| `-v` | Ctrl-C kills the emulator immediately |
| `-i` | Disable built-in commands (use only real OS-9 binaries) |
| `-m n[k\|M]` | Give the first process extra static storage |
| `-mm n[k\|M]` | Give all processes extra static storage |
| `-p prio` | Run first process at priority `prio` (default 128) |
| `-d[n] msk` | Set diagnostic trace mask (see `idbg` → `dh` for bit values) |
| `-r` | Run terminal output at full speed (disable baud-rate pacing) |
| `-h` | Full option list |

Console output is paced to the path's configured baud rate by default (see `tmode`) — a 300-baud session visibly trickles rather than dumping everything instantly, the way it would on real serial hardware. `-r` disables this for scripted/automated use.

---

## Built-in commands

When launched normally, os9exec intercepts a set of names before the OS-9 shell can fail on them. They run inside the emulator and look like regular OS-9 programs.

Some real OS-9 binaries assume an RBF file system and use low-level disk calls with no equivalent on a host-native directory (`mv` is the primary example — the real binary fails with `E_BMODE` on `/dd`). The built-in replacements work correctly regardless of whether the underlying path is a native directory or an RBF image. Pass `-i` to suppress all built-ins and use only real OS-9 binaries.

| Command | What it does |
|---------|--------------|
| `ihelp` / `icmds` | List all built-in commands |
| `iprocs` | Show running OS-9 processes |
| `imdir` | Show loaded OS-9 modules |
| `ipaths` | Show open paths |
| `imem` | Show memory blocks |
| `idevs` | Show mounted devices |
| `ihit` | Show directory hash hit rate (cache efficiency) |
| `idbg` / `debughalt` | Enter the emulator's interactive debugger |
| `dhelp` | List all debug/stop mask bit values (same as `idbg` → `dh`) |
| `stop` / `shutdown` | Exit os9exec cleanly |
| `rename` | Rename a file or directory |
| `move` / `mv` | Move files or directories (replaces RBF-only real `mv`) |
| `mount` / `unmount` | Mount, or create and mount, an RBF image or RAM disk at runtime |

**Tip:** A freeware `ls` (compiled from K&R C source using the Microware cc toolchain) is included in `dd/CMDS/SHARE/ls`. Directory listing via `fopen()` is a known limitation — use `dir` for directory contents.

---

## OS-9 `debug` now works

The OS-9/68k `debug` command — the interactive machine-level debugger that ships with every OS-9 SDK — works correctly under os9exec for the first time. Earlier versions implemented the debug syscalls (`F$DFork`, `F$DExec`, `F$DExit`) but the register display always showed zeroes, making the debugger useless.

```
$ debug /dd/CMDS/shell
- can't find 'shell.stb'.  Error #000:216 (E_PNNF) Path Name Not Found
no symbol module for address
dn: 00000004 00000000  00000080 00000003   DDDDDDD4 00000026  00001840 DDDDDDD7
an: AAAAAAA0 00023980  AAAAAAA2 00005580   AAAAAAA4 0002395A  0002A140 0002395A
pc: 000055CE  cc: 00 (-----)
<FPCP in Null state>
0x000055CE   >2D468014         move.l d6,-32748(a6)
dbg: gs
dn: FFFFFF04 00000000  00000080 00000003   DDDDDDD4 00000026  00001840 DDDDDDD7
an: 00022B52 00023980  AAAAAAA2 00005580   AAAAAAA4 0002395A  0002A140 00023956
pc: 000055D2  cc: 00 (-----)
dbg:
```

Single-step with `gs`, trace with `t <n>`, set breakpoints with `b`, display memory with `d`. Registers update live. Type `?` for the full command list.

<details>
<summary>How the fix was found</summary>

The debug syscalls work by having the debugger binary pass a pointer to its own register-frame buffer (in A2 at `F$DFork` time); the kernel writes the child's registers there after each step. Getting this right in a clean-room emulator required reconstructing the kernel's internal calling convention without access to the original Microware source.

The reconstruction used [*The OS-9 Guru, Book 1: The Facts*](https://www.icdia.co.uk/books_os9/os9guru/index.html) (Galactic Industrial Ltd., 1995; full scan on [Internet Archive](https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts)) as the primary source. OS-9 struct layouts were derived from the book's field descriptions and used to reconstruct the correct calling convention. The F$DFork handshake — specifically that A2 holds the parent's register-frame buffer address, stored by the kernel into `P$DbgReg` of the child's process descriptor — was confirmed from a fragment of the real kernel's `fork.a` assembly. The fix was then a small, precise change: capture `rp->a[2]` at DFork time, write child registers there in the correct 72-byte R$ frame layout, and inject `P$DbgPar` as a non-zero sentinel so the debugger binary recognises the child as being debugged.

All of this was done collaboratively with Claude Sonnet 4.6, which identified the root cause, cross-referenced the book against the kernel fragment, and wrote the fix.

</details>

---

## Emulator debugger (`idbg`)

Run `idbg` from the OS-9 shell to enter the emulator's own built-in debugger — separate from the OS-9 `debug` command above, this operates at the emulator level, inspecting the 68k state from the outside.

```
# Pid=3: dbgmsk=$0001,$0000,$0000 stop=$0000 trigger='' (type ?<Enter> for hlp)
```

| Command | Effect |
|---------|--------|
| `r` / `r xx` | Show 68k registers for current process / process `xx` |
| `i` / `i addr` | Disassemble 10 instructions from PC / from `addr` |
| `l` / `l addr` | Hex dump from A7 / from `addr` |
| `p` | List all OS-9 processes |
| `v` | Memory map for current process |
| `t` | Single-step one instruction |
| `x` | Resume |
| `k xx` | Kill process `xx` |
| `q` | Quit the emulator |
| `d mask` | Set diagnostic trace mask (`dh` to list bits) |
| `s mask` | Set stop mask — auto-enter debugger on matching events |

`s 1` is the most useful stop mask: it breaks into the debugger on any anomaly (bus error, address error, unimplemented syscall) at the exact point of failure.

---

## Command catalog

See [CMDS.md](CMDS.md) for the full list of known OS-9 commands with status notes.

## Compatibility

The full syscall surface — file I/O, process management, module loading, pipes, events, signals, traps, the shell — runs correctly.

**Time:** os9exec has no internal clock. `F$Time` delegates to the host, so `date` and file timestamps always reflect the host's system time. `setime` accepts a date but has no effect — the host clock is authoritative.

**Hardware-dependent commands** (`backup`, `format`, `tape`, `kermit`, raw `com`, `rdump`, `fsave`/`frestore`) require physical devices that are not emulated and will not work.

**Terminal I/O:** Full screen apps (`vi`, `less`, editors) require `TERM` to be set and a compatible termcap entry. The included `dd/SYS/termcap` covers `xterm`, `xterm-256color`, and `vt100`. `vi` is included in `dd/CMDS/SHARE` and is the recommended editor.

Everything else in a standard OS-9/68k SDK CMDS directory can be expected to run. See [CMDS.md](CMDS.md) for a command-by-command status list.

---

## Other ways to run it

The [3-step Quick start](#quick-start) above is the recommended path — it's one command (`make`) and gets you today's fixes. The [GitHub Releases](https://github.com/peacedudes/os9exec/releases) page still only has the older tagged `v0.0.0` binaries; no new release has been cut yet.

<details>
<summary>Download a binary instead of building</summary>

**These are the `v0.0.0` release binaries — older than this branch.**

- macOS ARM64 (M1/M2/M3): `os9exec-macos-arm64`
- macOS Intel: (available in releases)
- Linux 64-bit: `os9exec-linux-x64`
- Linux 32-bit: `os9exec-linux-i386`
- Windows: no tagged release binary yet — build from source or use Docker

```sh
mkdir -p dd/CMDS
cp /path/to/your/os9/CMDS/* dd/CMDS/
OS9DISK=$(pwd)/dd ./os9exec /dd/CMDS/shell
```

</details>

<details>
<summary>Docker (pre-built image)</summary>

```sh
docker pull ghcr.io/peacedudes/os9exec:latest
docker run -it -v /path/to/your/os9:/dd ghcr.io/peacedudes/os9exec:latest /dd/CMDS/shell
```

Or build locally:
```sh
git clone https://github.com/peacedudes/os9exec.git
cd os9exec-git_code
docker build -f docker/Dockerfile -t os9exec .
docker run -it -v /path/to/your/os9:/dd os9exec /dd/CMDS/shell
```

</details>

<details>
<summary>Apple Container (native macOS, macOS 26+)</summary>

```sh
# Install Apple Container from https://github.com/apple/container/releases
container system start

git clone https://github.com/peacedudes/os9exec.git
cd os9exec-git_code
container build -f docker/Dockerfile -t os9exec:apple .
container run -it -v /path/to/your/os9:/dd os9exec:apple /dd/CMDS/shell
```

</details>

<details>
<summary>Platform-specific build notes</summary>

- **macOS:** `make` (requires Xcode Command Line Tools)
- **Linux:** `make` (requires build-essential, clang/gcc)
- **Windows:** native build via [mingw-w64](https://www.mingw-w64.org/) — `make OS=Windows_NT CC=x86_64-w64-mingw32-gcc` (cross-compile from macOS/Linux, or run the same command natively in a Windows shell with mingw-w64 installed). Produces `os9exec.exe`. Docker and WSL2 remain available too.
- **Linux 32-bit:** `docker build -f Dockerfile.linux32 -t os9exec:linux32 .`

For an optimised build: `make prod`.

Four header files are required in `Source/OS9exec_core/os9defs/` to build: `module.h`, `procid.h`, `errno.h`, and `sgstat.h` (or symlinks to equivalents). These cover the core OS-9/68k struct layouts. Supply them from a licensed OS-9 system or derive them from *The OS-9 Guru* (Galactic Industrial) — see `Source/OS9exec_core/os9defs/defs_files.txt` for field-by-field guidance. No proprietary Microware source is required. The pre-built binary runs without them.

</details>

---

## Interactive REPL helper

`tools/os9repl.sh` wraps os9exec in a tmux session for scripted or automated use (including driving it from Claude):

```sh
./tools/os9repl.sh start            # launch
./tools/os9repl.sh send "dir /dd"   # send a command, get only new output
./tools/os9repl.sh peek             # see full pane (after a crash, etc.)
./tools/os9repl.sh restart          # rebuild + relaunch
./tools/os9repl.sh stop
```

Requires `tmux` (`brew install tmux`).

---

## Credits

Original OS9exec authors: Lukas Zeller, Beat Forster  
Original project: <http://www.synthesis.ch/os9exec>  
Source repository: <https://sourceforge.net/p/os9exec/git_code/ci/master/tree/>  
arm64 port and debug fix: Robert Doggett, with Claude Sonnet 4.6 (Anthropic)  
License: GNU General Public License v2 (see source file headers)

### Reference

*The OS-9 Guru, Book 1: The Facts* — Galactic Industrial Ltd., 1995.  
Landing page: <https://www.icdia.co.uk/books_os9/os9guru/index.html>  
Full scan: <https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts>

This book was the primary reference for reconstructing OS-9 kernel struct layouts and system call conventions used in the arm64 port. Galactic Industrial published it specifically to enable third-party OS-9 interoperability work.
