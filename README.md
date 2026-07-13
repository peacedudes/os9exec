# OS9exec — Complete OS-9/68k emulator

Run actual OS-9 binaries on your modern computer. OS9exec emulates the 68k processor and OS-9 kernel, giving you a real OS-9 shell with pipes, redirection, job control, and the complete filesystem. If you have OS-9 software sitting around, this is the fastest way back in.

**Platform support:** macOS (arm64/Intel), Linux (64-bit/32-bit), Windows (native, via mingw-w64), Docker (including on Windows, via Docker Desktop).

---

## Quick start

### Choose your path

**Recommended right now: build from source (Option 4).** The
[GitHub Releases](https://github.com/peacedudes/os9exec/releases) page
still only has the old `v0.0.0` binaries — this branch is meaningfully
ahead of that and no new release has been cut yet. Building takes one
command (`make`) and is the only way to get today's fixes until that
happens — especially on macOS, where it's one line with no dependencies
beyond Xcode Command Line Tools.

| Want | Use |
|------|-----|
| **Build from source (recommended for now)** | Option 4: `make` locally |
| **Have Docker installed** | Option 2: Pull pre-built image |
| **macOS 26+ with Apple Container** | Option 3: Native container tool |
| **Old tagged release binary** | Option 1: Download binary (stale, see above) |

### What you need

**OS9exec itself:** Build locally (see Option 4 below) — recommended
until a new release is cut. [GitHub Releases](https://github.com/peacedudes/os9exec/releases)
has older `v0.0.0` binaries if you'd rather not build.

**OS-9 software:** You must provide your own OS-9 binaries (shell, utilities, etc.). These are not included in the repo—they're not ours to distribute. If you have OS-9 software on disk/tape/archive, point os9exec at it. [Legitimate sources exist for hobbyists](https://www.icdia.co.uk/).

### Option 1: Download binary + bring your own OS-9

**These are the `v0.0.0` release binaries — older than this branch.** Use
[Option 4](#option-4-build-locally) instead unless you specifically want
that tagged release.

1. **Get the binary for your platform:**
   - macOS ARM64 (M1/M2/M3): `os9exec-macos-arm64`
   - macOS Intel: (available in releases)
   - Linux 64-bit: `os9exec-linux-x64`
   - Linux 32-bit: `os9exec-linux-i386`
   - Windows: no tagged release binary yet — use [Option 4](#option-4-build-locally) (`make OS=Windows_NT CC=x86_64-w64-mingw32-gcc`) or Option 2 (Docker), which runs as-is on Windows via Docker Desktop

2. **Set up your OS-9 files:**
   ```sh
   mkdir -p dd/CMDS
   cp /path/to/your/os9/CMDS/* dd/CMDS/
   ```

3. **Run:**
   ```sh
   OS9DISK=$(pwd)/dd ./os9exec /dd/CMDS/shell
   ```
   
   You're in OS-9! Try: `dir /dd/CMDS`, `echo hello`, `procs`, `exit`

### Option 2: Docker (pre-built image)

Pull the latest image:
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

### Option 3: Apple Container (native macOS, macOS 26+)

```sh
# Install Apple Container from https://github.com/apple/container/releases
container system start

git clone https://github.com/peacedudes/os9exec.git
cd os9exec-git_code
container build -f docker/Dockerfile -t os9exec:apple .
container run -it -v /path/to/your/os9:/dd os9exec:apple /dd/CMDS/shell
```

### Option 4: Build locally

```sh
git clone https://github.com/peacedudes/os9exec.git
cd os9exec-git_code
make
OS9DISK=/path/to/your/os9 ./os9exec /path/to/your/os9/CMDS/shell
```

**Platform-specific:**
- **macOS:** `make` (requires Xcode Command Line Tools)
- **Linux:** `make` (requires build-essential, clang/gcc)
- **Windows:** native build via [mingw-w64](https://www.mingw-w64.org/) —
  `make OS=Windows_NT CC=x86_64-w64-mingw32-gcc` (cross-compile from macOS/Linux)
  or run the same command natively in a Windows shell with mingw-w64 installed.
  Produces `os9exec.exe`. [Docker](#option-2-docker-pre-built-image) and WSL2
  remain available too.
- **Linux 32-bit:** `docker build -f Dockerfile.linux32 -t os9exec:linux32 .`

---

## macOS security note

First run may be blocked. Fix with:
```sh
xattr -d com.apple.quarantine os9exec
```
Or: System Settings → Privacy & Security → Allow


## The disk layout

OS-9 sees your files through a device called `/dd` (the "default drive"). There are
two ways to set it up — you can mix them freely:

**Native host directories** — the simplest approach. Just make a directory tree and
point `OS9DISK` at it. OS-9 programs see host files as OS-9 files. This is also the
easiest way to move files between OS-9 and the Mac: drop something in a host
subdirectory and it appears instantly inside the emulator, and vice versa.

```
dd/
  CMDS/       ← OS-9 executables
  SYS/        ← startup, password, etc.  (optional)
  DEFS/       ← header files             (optional)
  startup     ← auto-executed at boot    (optional)
```

**RBF disk images** — if you have an actual OS-9 disk image (from a real machine,
a tape backup, or an SDK archive), point `OS9DISK` directly at the image file.
The RBF file system is fully supported: `os9exec` mounts it read/write at `/dd`,
just as real OS-9 hardware would.

You can also attach extra host directories as `/h1`, `/h2`, etc. (see [Devices](#devices-and-host-filesystem)
below). This is useful for bridging an RBF image to the host — mount the image at
`/dd` and a host staging folder at `/h1`, then `copy /dd/myfile /h1/myfile` to extract.


## New: OS-9 `debug` works — for the first time ever

The OS-9/68k `debug` command — the interactive machine-level debugger that ships with
every OS-9 SDK — now works correctly under os9exec. This is new. Previous versions of
the emulator implemented the debug syscalls (`F$DFork`, `F$DExec`, `F$DExit`) but the
register display always showed zeroes, making the debugger useless.

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

Single-step with `gs`, trace with `t <n>`, set breakpoints with `b`, display memory
with `d`. Registers update live. Type `?` for the full command list.

### How the fix was found

The debug syscalls work by having the debugger binary pass a pointer to its own
register-frame buffer (in A2 at `F$DFork` time); the kernel writes the child's
registers there after each step. Getting this right in a clean-room emulator required
reconstructing the kernel's internal calling convention without access to the original
Microware source.

The reconstruction used [*The OS-9 Guru, Book 1: The Facts*](https://www.icdia.co.uk/books_os9/os9guru/index.html)
(Galactic Industrial Ltd., 1995; full scan on [Internet Archive](https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts))
as the primary source. OS-9 struct layouts were derived from the book's field
descriptions and used to reconstruct the correct calling convention. The F$DFork
handshake — specifically that A2 holds the parent's register-frame buffer address,
stored by the kernel into `P$DbgReg` of the child's process descriptor — was
confirmed from a fragment of the real kernel's `fork.a` assembly. The fix was then
a small, precise change: capture `rp->a[2]` at DFork time, write child registers
there in the correct 72-byte R$ frame layout, and inject `P$DbgPar` as a non-zero
sentinel so the debugger binary recognises the child as being debugged.

All of this was done collaboratively with Claude Sonnet 4.6, which identified the
root cause, cross-referenced the book against the kernel fragment, and wrote the fix.


## Run

```sh
OS9DISK=/path/to/disk-or-directory ./os9exec /dd/CMDS/shell
```

Press ESC on a blank line to exit the shell.


## Devices and host filesystem

| Device | How to set | What it maps to |
|--------|------------|-----------------|
| `/dd`  | `OS9DISK=…` env var, or a `dd` file/dir next to the binary | Default drive — RBF image or host directory |
| `/h0`–`/h9`, `/ha`–`/hz` | `OS9H0=…` through `OS9HZ=…`, or files/dirs named `h0`–`hz` next to the binary | RBF disk images or host directories |

Files placed in a host directory appear immediately inside the emulator as OS-9
files, with no conversion needed for binary modules. Text files need OS-9 line
endings (CR, `0x0D`) rather than Unix LF — the emulator handles this transparently
for `I$ReadLn`/`I$WritLn`, but raw byte copies preserve whatever endings are in
the file.

RBF disk images pointed to by `/h0`–`/hz` are auto-mounted on first access —
`dir /h0/CMDS` works directly without needing to access `/h0` first or run `mount`.
Use `mount <image> <devname>` to attach an image under a name of your choosing.

### Creating new disk images

`mount` can also create a brand-new device, instead of attaching an existing
one:

```
mount -r=<size> [<name>]      create an in-memory RAM disk, <size> in kBytes
mount -k=<size> h0..hz        create a ready-to-use blank RBF image on disk
mount -k=0      h0..hz        create a plain host directory instead
```

`<size>` accepts a bare number (bytes) or a `k`/`M`/`G` suffix (×1024/×1024²/×1024³) —
the same convention `os9exec`'s own `-m`/`-mm` command-line options use.

**RAM disk** (`-r=<size>`): fully formatted and usable immediately — no
`format` needed. Lives only in memory; gone on `unmount` or emulator exit.
`<name>` must be an absolute path (defaults to `/r0` if omitted) — it isn't
tied to the `h0`–`hz` convention since there's no host file involved, but it
does need the leading `/`.

```
mount -r=2000 /scratch    # 2000 kB RAM disk named /scratch
dir /scratch
unmount scratch           # releases the memory (bare name is fine here)
```

**Blank disk image** (`-k=<size>`): writes a fully formatted, ready-to-use
RBF image straight to `<dir-holding-the-binary>/hX` — the same place the
`/h0`–`/hz` auto-mount convention already looks (see the table above), so
the new device works immediately in the same session with no extra step.
`<size>` is rounded up to a valid sector/track/cluster boundary
automatically. The target must be `h0`–`hz` (never `dd`) and must not
already exist — `mount -k` refuses to overwrite an existing file or
directory.

```
mount -k=1M h7
dir /h7
```

To populate a freshly created image with real content (and verify the
copy), `dsave` from an existing directory works against it like any other
device — `-i` indents, `-v` verifies each file with `cmp`, `-e` executes
the generated script immediately instead of just printing it:

```
makdir /dd/USR/TESTER/doctest
echo hello from the mount -k example >/dd/USR/TESTER/doctest/hello.txt
chd /dd/USR/TESTER/doctest
dsave -ive /h7
```

**Device-resolution order, if you're layering these:** for any `/hX` path,
`os9exec` checks, in this order: (1) the `OS9Hx` environment variable, if
set; (2) a file/dir named `hX` next to the binary — what `mount -k` writes;
(3) one directory level up from the binary (a legacy fallback). Separately,
and taking priority over all three of those for as long as the current
process keeps running, an explicit `mount <file> <name>` (or `mount -r=`)
call registers a device directly in memory — if you've already mounted
something under a given name this session, a same-named `mount -k` file
created afterward is shadowed by that registration until the process
restarts.

### Devices stay inside their root (changed behavior)

Each device is a self-contained OS-9 volume. From inside the emulator you cannot
climb out of a device into the host filesystem: `..` at a device root resolves to
the root itself (like `/..` == `/` on Unix), and an absolute path that names no
configured device — `chd /usr`, `chd /etc` — is rejected rather than dropping you
into the real host directory. This matches how real OS-9/RBF hardware behaves: a
device root has no parent.

**This differs from earlier builds**, where a host-directory-backed device let
OS-9 walk `..` up into the host tree, `chd /usr` landed in the actual host `/usr`,
and `pd` printed the underlying host path (e.g. `/Users/you/.../dd/CMDS`) instead
of the OS-9 path (`/dd/CMDS`). That passthrough was convenient but not faithful to
OS-9 semantics — and a stray `del`/`makdir` on an unrecognized absolute path could
silently touch real host files. RBF disk images were always properly confined;
this brings host-directory devices in line with them.

If you *want* OS-9 to reach a specific host location, expose it deliberately as its
own device — symlink it to a device name next to the binary:

```
ln -s /any/host/dir h5      # now /h5 inside OS-9 is that directory
```

That keeps host access explicit and per-device instead of reachable by accident.


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

Console output is paced to the path's configured baud rate by default (see
`tmode`) — a 300-baud session visibly trickles rather than dumping
everything instantly, the way it would on real serial hardware. `-r`
disables this for scripted/automated use where realism doesn't matter.


## Built-in commands

When launched normally, os9exec intercepts a set of names before the OS-9 shell
can fail on them. They run inside the emulator and look like regular OS-9 programs.

Some real OS-9 binaries assume an RBF file system and use low-level disk calls that
have no equivalent on a host-native directory. `mv` is the primary example: the real
binary requires RBF directory-write access and fails with `E_BMODE` on `/dd`. The
built-in replacements work against the host file system directly, so they behave
correctly regardless of whether the underlying path is a native directory or an RBF
image. Pass `-i` to suppress all built-ins and use only real OS-9 binaries (note that
RBF-dependent commands will then fail on native paths).

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
| `mount` / `unmount` | Mount or unmount an RBF image at runtime |

Pass `-i` to disable all of these and use only real OS-9 binaries.

**Tip:** A freeware `ls` (compiled from K&R C source using the Microware cc toolchain)
is included in `dd/CMDS/SHARE/ls`. It lists files; directory listing via `fopen()` is
a known limitation (use `dir` for directory contents). The GNU fileutils 3.13 `ls` is
partially ported (`freeware/SRC/ls/*.r` compiled); linking is blocked pending cio/csl
trap handler resolution.


## Emulator debugger (`idbg`)

Run `idbg` from the OS-9 shell to enter the emulator's own built-in debugger.
This is separate from the OS-9 `debug` command above — it operates at the
emulator level, inspecting the 68k state from the outside.

```
# Pid=3: dbgmsk=$0001,$0000,$0000 stop=$0000 trigger='' (type ?<Enter> for hlp)
```

Key commands:

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

`s 1` is the most useful stop mask: it breaks into the debugger on any anomaly
(bus error, address error, unimplemented syscall) at the exact point of failure.


## Command catalog

See [CMDS.md](CMDS.md) for the full list of known OS-9 commands with status notes.


## Compatibility

The full syscall surface — file I/O, process management, module loading, pipes,
events, signals, traps, the shell — runs correctly.

**Time:** os9exec has no internal clock. `F$Time` delegates to the host, so `date`
and file timestamps always reflect the Mac's system time. `setime` accepts a date
but it has no effect — the host clock is authoritative and cannot be overridden from
inside the emulator. This is by design: keeping OS-9 time in sync with the host
filesystem avoids confusion with file modification timestamps.

**Hardware-dependent commands** (`backup`, `format`, `tape`, `kermit`, raw `com`,
`rdump`, `fsave`/`frestore`) require physical devices that are not emulated and
will not work.

**Terminal I/O:** Full screen apps (`vi`, `less`, editors) require `TERM` to be set
and a compatible termcap entry. The included `dd/SYS/termcap` covers `xterm`,
`xterm-256color`, and `vt100`. `vi` is included in `dd/CMDS/SHARE` and is the
recommended editor; it works correctly including insert mode and cursor movement.

Everything else in a standard OS-9/68k SDK CMDS directory can be expected to run.
See [CMDS.md](CMDS.md) for a command-by-command status list.


## Building from source

Requires Xcode Command Line Tools:

```sh
xcode-select --install
make
```

Produces `os9exec` in the repo root. For an optimised build: `make prod`.

Four header files are required in `Source/OS9exec_core/os9defs/` to build:
`module.h`, `procid.h`, `errno.h`, and `sgstat.h` (or symlinks to equivalents).
These cover the core OS-9/68k struct layouts. Supply them from a licensed
OS-9 system or derive them from *The OS-9 Guru* (Galactic Industrial) — see
`Source/OS9exec_core/os9defs/defs_files.txt` for field-by-field guidance.
No proprietary Microware source is required; the book's published descriptions
are sufficient. The pre-built binary runs without them.


## Interactive REPL helper

`tools/os9repl.sh` wraps os9exec in a tmux session for scripted or automated use
(including driving it from Claude):

```sh
./tools/os9repl.sh start            # launch
./tools/os9repl.sh send "dir /dd"   # send a command, get only new output
./tools/os9repl.sh peek             # see full pane (after a crash, etc.)
./tools/os9repl.sh restart          # rebuild + relaunch
./tools/os9repl.sh stop
```

Requires `tmux` (`brew install tmux`).


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

This book was the primary reference for reconstructing OS-9 kernel struct layouts
and system call conventions used in the arm64 port. Galactic Industrial published
it specifically to enable third-party OS-9 interoperability work.
