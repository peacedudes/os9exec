# OS9exec

An OS-9/68k emulator: a 68020 emulator, plus a reimplementation of the OS-9
kernel's system calls, so that unmodified OS-9 binaries run on macOS, Linux or
Windows.

The original authors' last release was V3.39, in 2007, when 32-bit hosts were
still the ordinary case. V4.0.0 (2026) brought it to 64-bit hosts: it builds and
runs on 32- and 64-bit machines, big-endian and little-endian alike.

**The current release is V4.1.1**: OS-9 networking programs reach the real
network, os9exec runs in a web browser, and a long list of fixes brings it
closer to the manuals. [What's new in V4.1](docs/release-notes-v4.1.0.md), and
the [two fixes in V4.1.1](docs/release-notes-v4.1.1.md).

**See it running, right now:
[open a live OS-9 system in your browser](https://peacedudes.github.io/osk-freeware/try/).**
That is os9exec, compiled to WebAssembly, booted from the freeware collection's
disk and waiting at a shell prompt. Nothing to install: type `dir`, or a
program's name - `rain`, `fortune`, `phoon`. Or open it with
[`banner1 -d -s OS-9` already typed](https://peacedudes.github.io/osk-freeware/try/?run=banner1%20-d%20-s%20OS-9),
or with [`rain`](https://peacedudes.github.io/osk-freeware/try/?run=rain), and press Enter.

**Looking for software to run?** The
[OS-9/68000 freeware collection](https://github.com/peacedudes/osk-freeware) is
three decades of community software for OS-9/68000 gathered on one disk image,
with source for most of it, ready to mount beside your system disk. It is also
how V4.1.0 was made: running that collection under os9exec found nearly every
fix since V4.0.0.

**Writing or porting OS-9 software with an AI assistant?** The
[OS-9 skills for AI coding assistants](https://github.com/peacedudes/os9-dev-skill)
are two agent skills that teach an assistant to work on Microware OS-9 for the
6809 and the 68000: BASIC09, Microware C, assembly, the shell and its
utilities, modules, system calls, error codes, and, below the application
line, device drivers, file managers and kernel internals. Each claim says
where it came from, and many were checked by running them: 68000 programs
under os9exec, 6809 programs on NitrOS-9 under XRoar.

### A continuation of the original authors' work

OS9exec was written by **Lukas Zeller** and **Beat Forster**. They both know of
this work, and are pleased to see os9exec given a new life on today's computers.
The emulator is their work: the [original project page](http://www.synthesis.ch/os9exec)
is still online, and V3.39 is still on [SourceForge](https://sourceforge.net/projects/os9exec/)
where they published it.

Everything since V3.39 is ours - as is any bug you may find in it.

---

## What you need

Any OS-9/68k software will do.

Have none? Start with the
[OS-9/68000 freeware collection](https://github.com/peacedudes/osk-freeware).
Its image is a complete disk of its own, with bash to log in to and over a
thousand programs to run. Download `osk-freeware.dd.gz` from its
[Releases](https://github.com/peacedudes/osk-freeware/releases), then:

```sh
gunzip osk-freeware.dd.gz
OS9DISK=$PWD/osk-freeware.dd OS9H0=$PWD/osk-freeware.dd os9exec -r bash /dd/SYS/login
```

Its [catalogue](https://peacedudes.github.io/osk-freeware/) has a card for
every program, and starts it in your web browser, on this emulator, with
nothing to install. Some programs want parts of a licensed OS-9 (its shell,
runb or compiler); the collection's README says how to attach yours.

The preferred format is an RBF disk image: a single binary file. A straight copy
of your normal boot disk is ideal, though a boot disk is not required - os9exec
does not boot. It is an ordinary C program, with no ROM, no device drivers and
no machine state to bring up.

OS9exec will also let any host directory masquerade as an RBF drive. Real OS-9
never did this; it's os9exec's answer to the problem every emulator has, which
is getting files in and out. There's no transfer step and nothing to mount: the
directory *is* the drive. Change a file on the host and OS-9 sees it at once;
write one from OS-9 and it's an ordinary file on your disk, in your editor, in
your backups.

It's imperfect: a host filesystem can't record owners and attributes the way RBF
does, record locking is unavailable, and disk-level commands like `format` and
`free` have nothing to work on. It's still the pleasant way to work. Just mirror
the standard OS-9 directory structure: a `CMDS` subdirectory for OS-9/68k
binaries, `SYS` for system files, and so on.

Without a disk, os9exec's built-in commands still work - they can create and
inspect an empty RBF image, for example. But the OS-9 shell is itself an OS-9
program, so `os9exec shell` needs a disk that carries one.

OS9exec comes with a self-test suite that exercises commands from the Microware
OS-9/68K SDK to show it is working as expected. Run it with `make test`;
[`docs/h0-setup.md`](docs/h0-setup.md) lists the 62 modules it expects, and
where they live on an SDK installation.

---

## Quick start

Nothing to install at all:
[run it in your browser](https://peacedudes.github.io/osk-freeware/try/). To run it on
your own machine:

```sh
git clone https://github.com/peacedudes/os9exec.git
cd os9exec
make
OS9DISK=/path/to/your/os9disk ./os9exec shell
```

`OS9DISK` points at an RBF disk image or a directory holding a `CMDS` folder of
OS-9 binaries. From within OS-9, that volume is known as `/dd`, the default drive.

```
$ dir /dd/CMDS
$ echo hello
$ procs
$ exit
```

For a prebuilt binary, Docker, or Apple Container, see
[Other ways to run it](#other-ways-to-run-it).

macOS quarantines anything downloaded from the internet, so a prebuilt binary
is refused on first run. Clear it, or approve it once in System Settings →
Privacy & Security:

```sh
xattr -d com.apple.quarantine os9exec
```

A binary you built yourself is not quarantined.

---

## What else changed

Since V3.39, as V4.0.0 released it; what V4.1.0 adds is in its
[release notes](docs/release-notes-v4.1.0.md).

Parts of OS-9 that were missing or approximated, and are now implemented:

- RBF record locking, including the end-of-file lock a reader following a
  writer waits on.
- File permissions. On an RBF image these are real: files carry their creator's
  identity and the checks are enforced. On a host directory they are enforced as
  far as the host allows, but ownership is not recorded, so every file reads as
  owned `0.0`. Previously the checks were skipped everywhere.
- `F$Event` - waiters are queued and woken as the manual describes.
- Host terminals: `OS9T1=pty` allocates one; `devs` reports which host device
  it landed on and the `screen` command to attach. With `tsmon /t1`, that is a
  second login session.
- OS-9's own `debug`: registers, single-step, breakpoints. See
  [OS-9 `debug` now works](#os-9-debug-now-works).

Other:

- Ctrl-C and Ctrl-E interrupt whatever is running.
- A bad pointer passed to a system call returns `E_BPADDR`, as OS-9 does,
  rather than taking the emulator down with it; a corrupt module is refused.
- Paths cannot climb out of `/dd` into the rest of the filesystem.
- A conformance suite checks the kernel's user-state system calls against the
  Microware manuals. It is run on macOS, Windows and Linux, and on emulated
  big-endian hardware.

Software that used to fail, and now runs:

- Editors, games and other curses programs - cursor positioning was corrupted
  by an LF→CRLF translation on the host side.
- The Microware GNU utilities - about 22 of them stopped at their first
  instruction on a bounds trap (`CHK2`/`CMP2`) the emulator got wrong.
- Packed BASIC09 modules under `runb`, blocked by `F$Link` and `F$Load` bugs.
  A packed module in the current directory no longer auto-runs; `load` it, or
  put it in an execution directory.
- `tsmon` → `login`, so a boot disk reaches a login prompt. `OS9STOP=1` lets a
  non-super login `stop`/`shutdown`.

The commit history has the rest.

---

## The disk layout

OS-9 sees your files through a device called `/dd` (the "default drive"). Two ways to set it up - mix them freely:

**Native host directories** - the simplest approach. Make a directory tree and point `OS9DISK` at it. OS-9 programs see host files as OS-9 files. This is also the easiest way to move files between OS-9 and the host: drop something in a host subdirectory and it appears instantly inside the emulator, and vice versa.

```
dd/
  CMDS/       ← OS-9 executables
  SYS/        ← startup, password, etc.  (optional)
  DEFS/       ← header files             (optional)
  startup     ← auto-executed at boot    (optional)
```

**RBF disk images** - if you have an actual OS-9 disk image (from a real machine, a tape backup, or an SDK archive), point `OS9DISK` directly at the image file. The RBF file system is fully supported: `os9exec` mounts it read/write at `/dd`, just as real OS-9 hardware would.

You can also attach extra host directories as `/h1`, `/h2`, etc. (see [Devices](#devices-and-host-filesystem) below) - useful for bridging an RBF image to the host: mount the image at `/dd` and a host staging folder at `/h1`, then `copy /dd/myfile /h1/myfile` to extract.

---

## Devices and host filesystem

| Device | How to set | What it maps to |
|--------|------------|-----------------|
| `/dd`  | `OS9DISK=…` env var, or a `dd` file/dir next to the binary | Default drive - RBF image or host directory |
| `/h0`–`/h9`, `/ha`–`/hz` | `OS9H0=…` through `OS9HZ=…`, or files/dirs named `h0`–`hz` next to the binary | RBF disk images or host directories |
| `/t1`–`/t49` | `OS9T1=…` through `OS9T49=…`, or `iterm` from inside the running system | Terminals. `pty` allocates one on first open, and `devs` reports the host device plus how to attach; a `/dev/…` path opens that terminal or serial port. Unset and undeclared means the device does not exist. |

Files placed in a host directory appear immediately inside the emulator as OS-9 files, with no conversion needed for binary modules. Text files need OS-9 line endings (CR, `0x0D`) rather than Unix LF. Nothing is translated: `I$ReadLn` stops only at CR, so an LF file reads as one long line. Convert host-edited text with `flip -m`.

**A host directory is not an RBF disk, and programs can fail on one in ways they never would on real OS-9**: no locking, no owners, permissions checked by the host, synthesized directory entries and sector numbers. [`docs/host-drives.md`](docs/host-drives.md) lists how host drives work and exactly how they fail.

**Record locking works on RBF images, not on host directories.** RBF
implements the full mechanism - a read on an update-mode path locks the record
it read, the next write releases it, a conflicting accessor sleeps, and a write
landing at end of file takes the EOF lock so a reader following a producer
waits at the edge instead of seeing a premature end of file. A host directory
has none of it. An explicit `SS_Lock` on a host *file* does report the truth -
it fails with `E$UnkSvc`, so a program that asks for a lock finds out it did
not get one. What is absent is the **automatic** locking, which is silent by
nature: nobody asked, so nobody is told.

This is deliberate, not an oversight. Host directories are a convenience
bridge with no counterpart on real OS-9, so there is no Microware behaviour to
be faithful to; they are already lossy for file attributes and ownership for
the same reason. A lock there could only ever be half-true anyway, since host
tools can change the file behind the emulator's back, whereas an RBF image is
opaque to the host. **If your program depends on record locking - including
the automatic read-lock/write-release that makes a read-modify-write cycle
safe - put the file in an RBF image.** On a host directory concurrent
read-modify-write can silently lose updates.

RBF disk images pointed to by `/h0`–`/hz` are auto-mounted on first access - `dir /h0/CMDS` works directly with no need to touch `/h0` first or run `mount`. Use `mount <image> <devname>` to attach an image under a name of your choosing.

### Creating new disk images

`mount` can also create a brand-new device, instead of attaching an existing one:

```
mount -r=<size> [<name>]      create an in-memory RAM disk, <size> in kBytes
mount -k=<size> h0..hz        create a ready-to-use blank RBF image on disk
mount -k=0      h0..hz        create a plain host directory instead
```

`<size>` accepts a bare number (bytes) or a `k`/`M`/`G` suffix (×1024/×1024²/×1024³) - the same convention `os9exec`'s own `-m`/`-mm` command-line options use.

**RAM disk** (`-r=<size>`): fully formatted and usable immediately - no `format` needed. Lives only in memory; gone on `unmount` or emulator exit. `<name>` must be an absolute path (defaults to `/r0` if omitted).

```
mount -r=2000 /scratch    # 2000 kB RAM disk named /scratch
dir /scratch
unmount scratch           # releases the memory (bare name is fine here)
```

**Blank disk image** (`-k=<size>`): writes a fully formatted, ready-to-use RBF image straight to `<dir-holding-the-binary>/hX` - the same place the `/h0`–`/hz` auto-mount convention already looks, so the new device works immediately in the same session with no extra step. Rounds `<size>` up to a valid sector/track/cluster boundary automatically. Target must be `h0`–`hz` (never `dd`) and must not already exist.

```
mount -k=1M h7
dir /h7
```

To populate a fresh image with real content (and verify the copy), `dsave` works against it like any other device:

```
makdir /dd/USR/CLAUDE/doctest
echo hello from the mount -k example >/dd/USR/CLAUDE/doctest/hello.txt
chd /dd/USR/CLAUDE/doctest
dsave -ive /h7
```

> **One limit worth knowing:** both `mount -r` and `mount -k` allocate from the emulator's own 68k memory arena (32MB by default, `-M` to change it) - not host RAM. Requesting a size near or above that ceiling is refused with a message saying how much of the arena is left.

**Device-resolution order, if you're layering these:** for any `/hX` path, `os9exec` checks, in order: (1) the `OS9Hx` environment variable, if set; (2) a file/dir named `hX` next to the binary - what `mount -k` writes; (3) one directory level up from the binary (a legacy fallback). An explicit `mount <file> <name>` (or `mount -r=`) call takes priority over all three for as long as the process keeps running.

**Terminals resolve directly, with no search.** Unlike `/hX` - which falls back through `OS9Hx`, then a file next to the binary, then one directory up - a terminal is bound only by its `OS9Tn` variable. Set means bound; unset means the device does not exist, and opening it returns `E_UNIT`. `OS9T1=pty` makes `os9exec` allocate a terminal; run `devs` to see the host device it landed on -- it prints the `screen` command to attach with. `OS9T1=/dev/cu.usbserial-1420` opens a real serial port. With `tsmon /t1` running inside OS-9, that terminal gets its own login prompt - one emulator, several independent sessions. You can also make one from inside a running system with `iterm t3`, the way `mount` makes an `/hX`; it prints the endpoint and the `screen` command straight away, so nothing has to be declared before start-up.

The port also runs at the speed OS-9 thinks it does: a bound terminal takes its rate from the path's own baud setting, and `tmode baud=2400` retunes a live one. That is invisible on a pty, which stores a speed without honouring it, but it is what makes a real serial cable work at the rate both ends agreed on.

A bare `OS9T` wildcard used to make every `/tN` you hadn't named spring up as a pty on first use. It was retired once `iterm` existed, because `iterm` is the case it was invented for and is better at it: the wildcard created a terminal rather than finding one, so it could never fail, and a typo like `/t5` for `/t4` silently handed you a terminal nobody was attached to. Now the refusal above always stands, and you make the extra terminal when you want it, by name.

A binding lasts for the life of the emulator, not the life of the path that opened it, so the device you attached to stays the same device between commands - and the terminal is 8-bit transparent, which is what lets `kermit` move a binary across it intact.

### Devices stay inside their root

Each device is a self-contained OS-9 volume. From inside the emulator you cannot climb out of a device into the host filesystem: `..` at a device root resolves to the root itself (like `/..` == `/` on Unix), and an absolute path that names no configured device is rejected rather than dropping you into a real host directory.

If you *want* OS-9 to reach a specific host location, expose it deliberately as its own device:

```
ln -s /any/host/dir h5      # now /h5 inside OS-9 is that directory
```

---

## Networking

OS-9 programs that use sockets reach the host's network directly: there is no
interface to configure and no stack to start, because the host owns both. Load
Microware's two database modules from your disk first, then run the programs as
on a real system:

```sh
load /dd/CMDS/BOOTOBJS/SPF/inetdb /dd/CMDS/BOOTOBJS/SPF/netdb_local
telnet some.host
```

Measured working: `telnet`, `ftp`, `ping`, `tcpsend`/`tcprecv`, `tftpd`,
`msend`/`mrecv`, RPC (`rsort` through `portmap`), the older socket library's
programs (`ttcp`, BIND's `nslookup` and `nsquery`), and the `telnetd` and `ftpd`
servers, which a client on the host can log in to. A server binds the same port
it would on OS-9, so `telnetd` wants port 23: macOS lets an ordinary user bind
that on all addresses, and on Linux the host has to allow low ports for users
(or you run it as root). Programs that configure interfaces (`ifconfig`,
`ipstart`) have nothing to do here. [CMDS.md](CMDS.md) lists the network tools.

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
| `-q[ms]` | Turn the 100Hz system tick off (`-q`), or retune it (`-q<ms>`) |
| `-r` | Run terminal output at full speed (disable baud-rate pacing) |
| `-l` | Sign on with one line (name, version, authors) instead of the full banner, e.g. for a page that embeds a small terminal |
| `-6` | Also open RBF disks without the OS-9/68000 format's "Cruz" mark, as 6809 (CoCo) disks are; off by default |
| `-W` | Debugging aid: a program's write outside its own memory (data area, requested blocks, loaded modules) is a bus error that names the address and instruction, as OS-9's SSM makes it on real hardware; off by default |
| `-h` | Full option list |

Console output is paced to the path's configured baud rate by default (see `tmode`) - a 300-baud session visibly trickles rather than dumping everything instantly, the way it would on real serial hardware. `-r` disables this for scripted/automated use.

---

## Built-in commands

When launched normally, os9exec intercepts a set of names before the OS-9 shell can fail on them. They run inside the emulator and look like regular OS-9 programs.

Some real OS-9 binaries assume an RBF file system and use low-level disk calls with no equivalent on a host-native directory (`mv` is the primary example - the real binary fails with `E_BMODE` on `/dd`). The built-in replacements work correctly regardless of whether the underlying path is a native directory or an RBF image. Pass `-i` to suppress all built-ins and use only real OS-9 binaries.

| Command | What it does |
|---------|--------------|
| `ihelp` / `icmds` | List all built-in commands |
| `iprocs` | Show running OS-9 processes |
| `imdir` | Show loaded OS-9 modules |
| `ipaths` | Show open paths |
| `imem` | Show memory blocks |
| `devs` / `idevs` | Show devices, and for a bound `/tN` the `screen` command to attach to it. Takes the bare name because Microware's `devs` reads a device table os9exec does not have; `/dd/CMDS/devs` still reaches theirs. |
| `ihit` | Show directory hash hit rate (cache efficiency) |
| `idbg` / `debughalt` | Enter the emulator's interactive debugger |
| `dhelp` | List all debug/stop mask bit values (same as `idbg` → `dh`) |
| `stop` / `shutdown` | Exit os9exec cleanly. Requires super-user; set `OS9STOP=1` in the host environment to let any logged-in account exit (handy when a `tsmon`/`login` session isn't super). |
| `rename` | Rename a file or directory |
| `pwd` | Print the current data directory, the way OS-9's own `pd` does -- for when the fingers type the other system's name. `pd` itself is left alone: the system disk's Microware utility is the one that should answer it. |
| `cd` | Change the current data directory, the way `chd` does. OS-9 has no `cd` because a forked program cannot move its caller's directory; os9exec is the kernel here, so this one can. `cd` with no argument prints where you are rather than going anywhere. |
| `move` / `mv` | Move files or directories (replaces RBF-only real `mv`) |
| `mount` / `unmount` | Mount, or create and mount, an RBF image or RAM disk at runtime |
| `iterm` | Make a `/tN` terminal at runtime, the way `mount` makes an `/hX`: `iterm t3` allocates a pty and prints the `screen` command to attach to it, `iterm t3 /dev/ttys004` uses a terminal you already have, and `iterm` alone lists what is bound. Before it, a `/tN` could only be named in `OS9T<n>`, before os9exec started. |

**Tip:** Directory listing via `fopen()` is a known limitation - use `dir` for directory contents, or an `ls` from your own disk. A GNU `ls` built for OS-9/68k is in the companion `osk-freeware` collection.

---

## OS-9 `debug` now works

The OS-9/68k `debug` command - the interactive machine-level debugger that ships with every OS-9 SDK - works correctly under os9exec for the first time. Earlier versions implemented the debug syscalls (`F$DFork`, `F$DExec`, `F$DExit`) but the register display always showed zeroes, making the debugger useless.

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

Step with `gs`, trace with `t <n>`, set breakpoints with `b`, display memory with `d`. Registers update live. Type `?` for the full command list.

### Two quirks of the shipped `debug` binary

These are bugs in Microware's own 1980s `debug`, not in os9exec - there is nothing to fix on our side, but they will waste your afternoon if you don't know them.

- **Set breakpoints by name (`b main`) - that works.** Do **not** take an address out of the `sc` symbol listing: `sc` double-counts the module's relocation, printing `real address + symbol offset`. Its addresses are wrong, and wrong by an amount that *grows* the deeper a symbol sits in the module, so the listing looks perfectly plausible. In one test `sc` placed `main` at an address that disassembles as `sprintf+0x22E`. The symbol *names* are fine; only the addresses are corrupt.
- **`gs` is not really a single-step.** It plants a temporary breakpoint at the fall-through address (`PC + instruction length`) and runs to it. So it steps *over* `bsr`/`jsr`, and across a *taken* branch it keeps going until that fall-through is reached anyway - a whole loop iteration later, or never, if the address is unreachable (dead code after a `bra`). Prefer `b <name>` + `g` to land on a chosen spot. A runaway is always recoverable with **Ctrl-C**, which returns you to a fresh `dbg:` prompt.

When in doubt, disassemble an address (`di <addr>`) before trusting it - a function entry should look like a prologue.

<details>
<summary>How the fix was found</summary>

The debug syscalls work by having the debugger binary pass a pointer to its own register-frame buffer (in A2 at `F$DFork` time); the kernel writes the child's registers there after each step. Getting this right in a clean-room emulator required reconstructing the kernel's internal calling convention without access to the original Microware source.

The reconstruction used [*The OS-9 Guru, Book 1: The Facts*](https://www.icdia.co.uk/books_os9/os9guru/index.html) (Galactic Industrial Ltd., 1995; full scan on [Internet Archive](https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts)) as the primary source. OS-9 struct layouts were derived from the book's field descriptions and used to reconstruct the correct calling convention. The F$DFork handshake - specifically that A2 holds the parent's register-frame buffer address, stored by the kernel into `P$DbgReg` of the child's process descriptor - was confirmed from a fragment of the real kernel's `fork.a` assembly. The fix was then a small, precise change: capture `rp->a[2]` at DFork time, write child registers there in the correct 72-byte R$ frame layout, and inject `P$DbgPar` as a non-zero sentinel so the debugger binary recognises the child as being debugged.

All of this was done collaboratively with Claude Sonnet 4.6, which identified the root cause, cross-referenced the book against the kernel fragment, and wrote the fix.

</details>

---

## Emulator debugger (`idbg`)

Run `idbg` from the OS-9 shell to enter the emulator's own built-in debugger - separate from the OS-9 `debug` command above, this operates at the emulator level, inspecting the 68k state from the outside.

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
| `s mask` | Set stop mask - auto-enter debugger on matching events |

`s 1` is the most useful stop mask: it breaks into the debugger on any anomaly (bus error, address error, unimplemented syscall) at the exact point of failure.

---

## Command catalog

See [CMDS.md](CMDS.md) for the full list of known OS-9 commands with status notes.

## The conformance disk: check os9exec, or a real OS-9 machine

`conf68k.dsk`, attached to each [release](https://github.com/peacedudes/os9exec/releases),
holds 116 small tests of OS-9/68000 behaviour, each tied to a passage in
Microware's manuals. They are hand-written 68000 assembly with no Microware
software on the disk, so it can go anywhere.

- **Under os9exec, on any platform, with no OS-9 system disk:**

  ```sh
  OS9DISK=/full/path/to/conf68k.dsk os9exec -r /dd/CMDS/run
  ```

  On Windows: `set OS9DISK=C:/full/path/to/conf68k.dsk`, then
  `os9exec -r /dd/CMDS/run`.
- **On real OS-9/68000 hardware:** put the image on a disk, and as the super
  user `chd` to it, `chx` to its `CMDS`, and run `runall >>+RESULTS/errors`.

At the end it says in plain words whether there is anything to send. Only a
failure is news: it then asks you to
[open an issue](https://github.com/peacedudes/os9exec/issues) with
`RESULTS/report` (and `RESULTS/errors`, if there is one). A SKIP is not a
failure; it means that claim cannot be checked on your system or disk. A failure on real hardware is the most useful
report this project can receive, because it is the one thing an emulator
cannot tell us for itself. The disk's `readme` has the details.

## Checking a build

One command runs every gate this project has and prints one verdict:

```sh
make verify         # host + Linux (docker)
make verify-vms     # ... and the real virtual machines
make verify-quick   # host only, no containers
```

It exits non-zero if anything failed, so it works as a pre-commit gate rather
than something to read. The individual pieces are still there if you want one
of them on its own - `make test`, `make test-notick`, `make warnings`,
`make live-verify`, `make hammer`, `make test-linux`, and
`tools/conformance.sh 68k [--noshell --rbf]`.

The conformance suite also builds as a single disk you can carry to real
hardware:

```sh
make selfhost68k          # build/selfhost68k/conf68k.dsk
make selfhost68k-verify    # every file byte-checked, then all tests run off it
```

See `test/68k-conformance/readme` for how to run that disk on a real OS-9/68000
system.

## Compatibility

The full syscall surface - file I/O, process management, module loading, pipes, events, signals, traps, the shell - runs correctly.

**Time:** os9exec has no internal clock. `F$Time` delegates to the host, so `date` and file timestamps always reflect the host's system time. `setime` accepts a date but has no effect - the host clock is authoritative.

**File permissions:** enforced on **RBF disk images** (including RAM disks, `mount -r` - same filesystem code, just backed by memory instead of a host file). A file/directory is stamped with its creator's `group.user` at creation, and owner/public read-write-execute bits are checked on open, create, delete, and `attr` changes - with an unconditional super-user bypass, matching real OS-9.

Host-native devices (a plain host folder mapped as an OS-9 filesystem, including `OS9DISK` itself) have no on-disk file-descriptor sector to hold an owner byte - ownership isn't tracked there (every file reads as owned `0.0`), and there's no software bypass for super-user the way RBF has one. But `attr`'s read/write/execute bits *are* mapped to a real host permission change, as faithfully as each platform allows:
- **Unix (macOS/Linux):** owner and public map independently to real `chmod` bits (owner → `u`, public → `g`+`o`), the same for read, write, and execute. A file with no read bits genuinely can't be read back - including by `os9exec` itself - until `attr +r`/`+pr` restores it, the same as any other Unix process.
- **Windows:** write is a single, host-wide flag (`FILE_ATTRIBUTE_READONLY`) - there's no separate owner-vs-public write on this platform, so clearing only one side still leaves the file writable. Read denial uses a real NTFS ACL (`DENY` for `Everyone`), which genuinely blocks access - but mingw's own `stat()` doesn't consult ACLs, so `attr`'s *display* won't reflect a read denial even though it's really enforced underneath. Execute isn't real there either way - NTFS has no per-file executable bit, so a host-native file always reads as executable on Windows regardless of its actual state.

If you need OS-9 ownership semantics (not just read/write/execute enforcement) to mean something, use an RBF image (`mount -k=<size>`).

Owner `0.0` is intentional, not unfinished - there's nowhere on a host-native device to store a real one. A file with no public-write bit is only editable as super-user, even by the account that created it (e.g. your own `.login`); add `attr +pw` if you want it self-editable. That same fake `0.0` owner also means `copy`/`dsave` from a host-native source onto an RBF image only carries its attributes over when the copying process is super-user; otherwise the RBF copy keeps its own default attributes and needs `attr` run by hand afterward.

**Hardware-dependent commands** (`backup`, `format`, `tape`, `kermit`, raw `com`, `rdump`, `fsave`/`frestore`) require physical devices that are not emulated and will not work.

**Terminal I/O:** Full screen apps (`vi`, `less`, editors) require `TERM` to be set and a compatible termcap entry. Your disk needs a `SYS/termcap` covering your terminal; the one in the companion `osk-freeware` collection covers `xterm`, `xterm-256color` and `vt100`. An editor is not part of os9exec; `vi`, `sedt` and others are in the companion `osk-freeware` collection.

Everything else in a standard OS-9/68k SDK CMDS directory can be expected to run. See [CMDS.md](CMDS.md) for a command-by-command status list.

---

## Other ways to run it

The [3-step Quick start](#quick-start) is the recommended path: one `make`, and you get everything described above. Prebuilt binaries for each tagged release are on the [GitHub Releases](https://github.com/peacedudes/os9exec/releases) page.

<details>
<summary>Download a binary instead of building</summary>

- macOS ARM64 (M1/M2/M3): `os9exec-macos-arm64`
- Linux 64-bit: `os9exec-linux-x64`
- Linux 32-bit: `os9exec-linux-i386`
- Windows 64-bit: `os9exec-windows-x64.exe`

There is no macOS Intel binary. The build works and is tested, but nothing
publishes it: CI runs on Apple Silicon, and an Intel binary built there cannot
be smoke-tested on the same machine. Build it yourself with
`make CC="cc -arch x86_64" prod`, which is what we test.

```sh
mkdir -p dd/CMDS
cp /path/to/your/os9/CMDS/* dd/CMDS/
OS9DISK=$(pwd)/dd ./os9exec /dd/CMDS/shell
```

</details>

<details>
<summary>In a web browser (WebAssembly)</summary>

os9exec builds to WebAssembly and runs in a browser, with a terminal in the
page, from one disk image you supply:

```sh
brew install emscripten                       # or install emsdk yourself
tools/wasm-web.sh /path/to/disk.dsk bash /dd/SYS/login
python3 -m http.server -d build/web 8000      # then open http://localhost:8000
```

The disk is gzipped into `build/web` and inflated in the page, so a 328 MB
image is a 49 MB download. The page can also open a second image of your own as
`/h1` ("Open disk as /h1..."), which it keeps in the browser's own storage so it
survives a reload, and "Save /h1" downloads it again. A file you open this way
never leaves your machine: the page has no network code.

`?run=<command>` types a command once the system has started, which is what a
"try it" link uses. It is typed, not run: the reader presses Enter.

What this build does not have: sockets, so nothing networked runs (a browser
cannot open TCP at all, and socket paths say so); and host directories, so a
disk must be an image. The system tick is on, as everywhere else, so a program
that computes without system calls does not freeze the page.

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
cd os9exec
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
cd os9exec
container build -f docker/Dockerfile -t os9exec:apple .
container run -it -v /path/to/your/os9:/dd os9exec:apple /dd/CMDS/shell
```

</details>

<details>
<summary>Platform-specific build notes</summary>

- **macOS:** `make` (requires Xcode Command Line Tools)
- **Linux:** `make` (requires build-essential, clang/gcc)
- **Windows x86_64:** native build via [mingw-w64](https://www.mingw-w64.org/) - `make OS=Windows_NT CC=x86_64-w64-mingw32-gcc` (cross-compile from macOS/Linux, or run the same command natively in a Windows shell with mingw-w64 installed). Produces `os9exec.exe`. Docker and WSL2 remain available too. What the Windows build does not have: networking (socket paths answer `E$Unit`) and host terminals for `/tN`; for either, use Docker or WSL2. Everything else is the same, the system tick included, and it is tested on Windows 11: the conformance suite, and standard input redirected from a file, a pipe and `NUL`.
- **Windows ARM64 (native, not emulated x64):** confirmed working with two common clang-based toolchains, both requiring zero source changes - `make CC=clang` from an [MSYS2](https://www.msys2.org/) `CLANGARM64` shell (`pacman -S mingw-w64-clang-aarch64-toolchain make`), or the standalone [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) distribution (`make CC=aarch64-w64-mingw32-clang`). Visual Studio's `clang-cl`/MSVC toolchain does **not** currently work - it needs a `dirent`/`termios` compatibility layer against Win32 that doesn't exist yet (the codebase had one once, `msdir.c`/`msdir.h`, removed before this branch).
- **Windows 32-bit (x86):** `make OS=Windows_NT CC=i686-w64-mingw32-gcc`. Built warning-clean on every `make warnings` run, and now run as well: the conformance suite passes on it (all 116 tests from `conf68k.dsk`: 115 pass, and the last skips only because `load` is Microware's and not on the disk), executed on Windows 11 ARM64 through its x86 emulation. The 32-bit toolchain catches things the 64-bit one cannot - it found a pointer-width bug and a `__stdcall` bug the day it was added.
- **Linux 32-bit:** `docker build -f Dockerfile.linux32 -t os9exec:linux32 .`

For an optimised build: `make prod`.

Four header files are required in `Source/OS9exec_core/os9defs/` to build: `module.h`, `procid.h`, `errno.h`, and `sgstat.h` (or symlinks to equivalents). These cover the core OS-9/68k struct layouts. Supply them from a licensed OS-9 system or derive them from *The OS-9 Guru* (Galactic Industrial) - see `Source/OS9exec_core/os9defs/defs_files.txt` for field-by-field guidance. No proprietary Microware source is required. The pre-built binary runs without them.

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

### NitrOS-9 (6809) REPL

`tools/nitros9repl.sh` provides the same interface against a *real NitrOS-9
system* running on an emulated CoCo3 - XRoar's "becker port" tunnels
DriveWire over TCP to a DriveWire server, and the guest's own `inetd` asks
that server to listen on a port and forks a login onto each connection:

```sh
./tools/nitros9repl.sh start            # boot server + XRoar to a shell (~40s)
./tools/nitros9repl.sh send mdir        # send a command, get only new output
./tools/nitros9repl.sh connect          # interactive session in your terminal
./tools/nitros9repl.sh stop
```

Prerequisites beyond this repo: `xroar` (brew), a CoCo3 ROM, the NitrOS-9
EOU `dw_becker` disk image, and `drivewire-cli` built from stock
[drpitre/drivewire](https://github.com/drpitre/drivewire) `main` - no patched
build is needed, since both halves of the `tcp listen`/`tcp join` protocol
ship upstream. The disk needs `inetd&` in its `startup` and a matching port
line in `SYS/inetd.conf`. The script header documents paths and environment
overrides.

---

## Credits

**OS9exec was created by Lukas Zeller and Beat Forster**, 1993–2007, and is theirs. Everything below stands on their work.

Original project: <http://www.synthesis.ch/os9exec>  
Their final release: **V3.39**, 11 May 2007 - archived at <https://sourceforge.net/projects/os9exec/> (historical; this fork does not publish there)  
This continuation: <https://github.com/peacedudes/os9exec> - Robert Doggett, with Claude (Anthropic)  
Its siblings: the [OS-9/68000 freeware collection](https://github.com/peacedudes/osk-freeware), whose programs found nearly every fix in V4.1.0, and the [OS-9 skills for AI coding assistants](https://github.com/peacedudes/os9-dev-skill), whose 68000 examples were checked by running them under os9exec  
License: GNU General Public License v2 or later (see source file headers)

Work in this fork: ports to Apple Silicon, modern Linux and native Windows; verification on riscv64 and on big-endian s390x and sparc64; a 68k conformance suite run against the published manuals; and fixes across the syscall surface, RBF record locking, terminal I/O and the scheduler. About 1,250 commits on top of V3.39 - see [What else changed](#what-else-changed) and the [V4.1.0 release notes](docs/release-notes-v4.1.0.md). Why things are the way they are, including what was deliberately left alone, is in [`docs/decisions.md`](docs/decisions.md).

### Reference

*The OS-9 Guru, Book 1: The Facts* - Galactic Industrial Ltd., 1995.  
Landing page: <https://www.icdia.co.uk/books_os9/os9guru/index.html>  
Full scan: <https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts>

This book was the primary reference for reconstructing OS-9 kernel struct layouts and system call conventions used in the arm64 port. Galactic Industrial published it specifically to enable third-party OS-9 interoperability work.
