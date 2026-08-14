# OS9exec — Complete OS-9/68k emulator

Run real OS-9/68k binaries on a modern machine. OS9exec emulates the 68000 and the OS-9 kernel — a genuine OS-9 shell with pipes, redirection, job control, and a full filesystem.

**Platforms:** macOS (arm64/Intel), Linux (64-bit/32-bit), Windows (native x86_64, ARM64 and x86, via mingw-w64/MSYS2/llvm-mingw), Docker. Also built and test-verified on riscv64 and on two big-endian hosts, s390x and sparc64.

> ### A continuation, with the original authors' blessing
>
> OS9exec was written by **Lukas Zeller** and **Beat Forster**, who released it under the GPL and last published **V3.39 in 2007**. Both were contacted in August 2026, and both replied.
>
> **Lukas Zeller** endorses this continuation on GitHub, and does not wish to resume the project himself. **Beat Forster** has no objection to it being published here, and intends to try it on macOS, Linux and Windows.
>
> The emulator is their work. Everything since V3.39 is ours — as is any bug you find in it.

---

## Quick start

```sh
git clone https://github.com/peacedudes/os9exec.git
cd os9exec
make
OS9DISK=/path/to/your/os9disk ./os9exec shell
```

That's it: **1)** build it, **2)** point `OS9DISK` at a directory (or disk image) with a `CMDS` folder full of OS-9 binaries, **3)** run. From within OS-9 the disk is known as `/dd`.

```
$ dir /dd/CMDS
$ echo hello
$ procs
$ exit
```

Don't have OS-9 software yet? Bring your own — a disk image, tape backup, or SDK archive. OS9exec doesn't include any (not ours to distribute). If you have a Microware OS-9 for 68K SDK, [`docs/h0-setup.md`](docs/h0-setup.md) lists the 62 modules the test suite needs and where they live on it.

Prefer a prebuilt binary, Docker, or Apple Container instead of building? See [Other ways to run it](#other-ways-to-run-it) below.

---

## What's new

Since the tagged `v0.0.0`, this branch fixes a large class of crashes, hangs, and boots-to-nowhere. What a returning OS-9 user will notice:

**More real software runs**
- **The GNU utilities work.** ~22 tools from the Microware GNU archive died at their first instruction on a spurious bounds trap (`CHK2`/`CMP2`) — fixed; they launch and run.
- **Packed BASIC09 modules run under `runb`** (F$Link/F$Load wildcard and access-mode bugs fixed). *Behavior change:* a packed module in the current directory no longer auto-runs — `load` it, or put it in an execution directory.
- **BASIC09 can call compiled C** through a small assembly shim.
- **`ls` and `ls -l`** list directories correctly, given a disk that carries an `ls`.

**Boots and interacts cleanly**
- **`tsmon` → `login` boot reaches a prompt.** A keypress now delivers the data-ready signal `tsmon` blocks on, and the idle scheduler no longer deadlocks on an empty run queue. Set `OS9STOP=1` so a plain (non-super) login can still `stop`/`shutdown`.
- **Ctrl-C / Ctrl-E interrupt anything**, whatever the process is doing (previously a no-op outside a few states).
- **`dir` no longer hangs** on certain directories.
- **Full-screen apps position the cursor correctly** — a stray host LF→CRLF translation had broken cursor motion for `vi` and curses programs.

**Disks and devices**
- **`mount -k=<size>`** creates a ready-to-use blank RBF image (`-k=0`: a plain host folder) in one command — no separate `format` step. See [Creating new disk images](#creating-new-disk-images).
- **`mount -r=<size>` RAM disks** now actually work — silently broken in every prior build.
- **RBF file permissions are enforced for real** — files/dirs are stamped with their creator's identity at creation, and owner/public read-write-execute bits are checked on open, create, delete, and `attr` (super-user bypasses, as on real OS-9). Every file used to be owned `0.0` with every check skipped, regardless of attribute bits.
- **Host-native permissions are enforced for real too, best-effort per platform** — `attr` on a plain host folder (`mount -k=0`, or `OS9DISK` itself) now maps owner/public read-write-execute bits to a genuine host `chmod` (Unix) or read-only attribute + ACL (Windows), instead of being a silent no-op. There's no super-user exemption here: unlike RBF's bypass (a software check in `os9exec`'s own code), a real host permission is enforced by the host kernel, which has no concept of OS-9 privilege — even the file's own creator needs `attr +r`/`+pr` to read it back after clearing read. `copy`/`dsave`'s default FD-duplication now actually carries attributes (previously dates only) onto a host-native destination. See [Compatibility](#compatibility) for exact platform limits.
- **Devices stay inside their root** — no path can climb out of `/dd` into the host filesystem.
- **`OS9DISK` / `OS9Hx` paths containing `./` or `../` work** — they used to silently redirect every file access to the device root.
- **`/dd` reaches an RBF image no matter where `os9exec` is started.** Run from `/` — or from any directory one level below it, which is where containers usually run — the emulator's search for a `dd` disk beside itself built the string `/dd`, which is a *device* name rather than a host path. That mounted the image under the name `h0`, so the real `/dd` was then refused as busy and every `/dd/...` lookup failed (`E$PNNF`, or `E$UNIT` for a bare name) while the identical image reached through `/h7` worked.

**Inter-process signalling**
- **`F$Event` now has a real event queue.** Waiters are held FIFO and woken by value, instead of the previous approximation — so `Ev$Wait`, `Ev$Signal` and `Ev$Pulse` behave as the manual describes when more than one process is waiting. **`Ev$Pulse`** (wake whoever qualifies right now, then restore the value) is implemented, and **`Ev$Delet` refuses to delete an event that is still linked** (`E$EvBusy`) rather than deleting it under its users. 13 of the 218 modules in a stock SDK `CMDS` call `F$Event`.

**Windows**
- **`OS9DISK` written with backslashes now works.** `OS9DISK=C:\os9\disk` found nothing at all — every path lookup failed and every test reported `E$PNNF` — because only forward slashes were ever translated. Backslashes are now normalised at the single point where a device root is produced, so the native spelling of a Windows path just works.
- **Non-interactive runs no longer hang.** Piped stdin (scripts, CI, `echo cmds | os9exec shell`) went unread entirely — every non-interactive Windows run just sat there.
- **Drive-letter paths (`C:/...`) no longer get corrupted** by the `../` path-collapse logic, which assumed every path starts with a `/` the way Unix paths do.
- **A UAE-internal macro leak fixed for real:** the emulator core's own Amiga-disk-image attribute flags were shadowing the standard POSIX `stat()` bit names on MINGW, silently breaking both file-attribute reads and directory detection.
- **A symlink could escape a device's configured root** on native Windows; confinement is now re-checked against the resolved path.

**More robust**
- **OS-9's own `debug` command works for the first time** — live register display, single-stepping, breakpoints. See [OS-9 `debug` now works](#os-9-debug-now-works).
- **A buggy program can't crash the emulator through a syscall** — a wild or null pointer handed to a system call now returns `E_BPADDR`, as real OS-9 would, instead of faulting the host.
- **A corrupt or hostile module file can't crash the loader** — a module whose internal name/data offsets point outside itself is rejected (`E_BMID`/`E_FNA`) instead of reading or writing host memory it shouldn't; sync word, parity and CRC alone were never enough to trust those offsets. An over-long path is likewise refused (`E_BPNAM`) instead of overrunning a buffer.
- **Console baud-rate pacing** — output trickles at the path's configured baud rate like real serial hardware; `-r` restores full speed for scripts.

**More platforms**
- **Native Windows** (mingw-w64) and **ARM / 32-bit Linux** now run. ARM Linux was completely broken — an unsigned-`char` assumption made every file read hang.
- **Big-endian works, for the first time since the PowerPC era.** os9exec had only ever run where the OS-9 byte swaps are effectively no-ops. Bringing it up on **s390x** and **sparc64** exposed two real bugs: Linux was assumed little-endian outright, and the register-half macros hardcoded 32-bit offsets, so on a 64-bit big-endian host every syscall read its trap vector from the wrong 16 bits. Both fixed; the conformance suite now passes identically on little- and big-endian.
- **Verified on the machines, not just cross-compiled.** The suite is run on real macOS, Linux and Windows, plus riscv64, s390x and sparc64 — same results on every one (31 pass / 13 skip host-native, 44/44 on an RBF image). All six build warning-free.

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
| `/t1`–`/t49` | `OS9T1=…` through `OS9T49=…`, or the wildcard `OS9T=…` for any `/tN` not named individually | Terminals. `pty` allocates one and prints the device to attach to; a `/dev/…` path opens that terminal or serial port. Unset (and no wildcard) means the device does not exist. |

Files placed in a host directory appear immediately inside the emulator as OS-9 files, with no conversion needed for binary modules. Text files need OS-9 line endings (CR, `0x0D`) rather than Unix LF — the emulator handles this transparently for `I$ReadLn`/`I$WritLn`, but raw byte copies preserve whatever endings are in the file.

**Record locking works on RBF images, not on host directories.** RBF
implements the full mechanism — a read on an update-mode path locks the record
it read, the next write releases it, a conflicting accessor sleeps, and a write
landing at end of file takes the EOF lock so a reader following a producer
waits at the edge instead of seeing a premature end of file. A host directory
has none of it. An explicit `SS_Lock` on a host *file* does report the truth —
it fails with `E$UnkSvc`, so a program that asks for a lock finds out it did
not get one. What is absent is the **automatic** locking, which is silent by
nature: nobody asked, so nobody is told.

This is deliberate, not an oversight. Host directories are a convenience
bridge with no counterpart on real OS-9, so there is no Microware behaviour to
be faithful to; they are already lossy for file attributes and ownership for
the same reason. A lock there could only ever be half-true anyway, since host
tools can change the file behind the emulator's back, whereas an RBF image is
opaque to the host. **If your program depends on record locking — including
the automatic read-lock/write-release that makes a read-modify-write cycle
safe — put the file in an RBF image.** On a host directory concurrent
read-modify-write can silently lose updates.

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
makdir /dd/USR/CLAUDE/doctest
echo hello from the mount -k example >/dd/USR/CLAUDE/doctest/hello.txt
chd /dd/USR/CLAUDE/doctest
dsave -ive /h7
```

> **One limit worth knowing:** both `mount -r` and `mount -k` allocate from the emulator's own 68k memory arena (32MB by default, `-M` to change it) — not host RAM. Requesting a size near or above that ceiling fails with "No more memory."

**Device-resolution order, if you're layering these:** for any `/hX` path, `os9exec` checks, in order: (1) the `OS9Hx` environment variable, if set; (2) a file/dir named `hX` next to the binary — what `mount -k` writes; (3) one directory level up from the binary (a legacy fallback). An explicit `mount <file> <name>` (or `mount -r=`) call takes priority over all three for as long as the process keeps running.

**Terminals resolve directly, with no search.** Unlike `/hX` — which falls back through `OS9Hx`, then a file next to the binary, then one directory up — a terminal is bound only by its `OS9Tn` variable. Set means bound; unset means the device does not exist, and opening it returns `E_UNIT`. `OS9T1=pty` makes `os9exec` allocate a terminal and print the device to attach to (`screen /dev/ttys004`); `OS9T1=/dev/cu.usbserial-1420` opens a real serial port. With `tsmon /t1` running inside OS-9, that terminal gets its own login prompt — one emulator, several independent sessions.

The port also runs at the speed OS-9 thinks it does: a bound terminal takes its rate from the path's own baud setting, and `tmode baud=2400` retunes a live one. That is invisible on a pty, which stores a speed without honouring it, but it is what makes a real serial cable work at the rate both ends agreed on.

There's also a bare `OS9T` wildcard, for when you want every `/tN` you haven't named individually to spring up as a pty on first use: `OS9T=pty ./os9exec shell` and `/t1`, `/t2`, and so on each allocate the moment something opens them, no per-device variable needed. It's deliberately opt-in. Without it the refusal above stands exactly as described — a typo like `/t5` for `/t4` still reports `E_UNIT` rather than silently handing you a terminal nobody is attached to — and a per-device `OS9Tn` always takes priority over the wildcard, including its failures: if you name a real serial port that turns out to be missing, you get that error, not a surprise pty. The two combine freely, e.g. `OS9T=pty OS9T1=/dev/cu.usbserial-1420` puts real hardware on `/t1` and lets every other terminal allocate on demand.

A binding lasts for the life of the emulator, not the life of the path that opened it, so the device you attached to stays the same device between commands — and the terminal is 8-bit transparent, which is what lets `kermit` move a binary across it intact.

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
| `stop` / `shutdown` | Exit os9exec cleanly. Requires super-user; set `OS9STOP=1` in the host environment to let any logged-in account exit (handy when a `tsmon`/`login` session isn't super). |
| `rename` | Rename a file or directory |
| `move` / `mv` | Move files or directories (replaces RBF-only real `mv`) |
| `mount` / `unmount` | Mount, or create and mount, an RBF image or RAM disk at runtime |

**Tip:** Directory listing via `fopen()` is a known limitation — use `dir` for directory contents, or an `ls` from your own disk. A GNU `ls` built for OS-9/68k is in the companion `osk-freeware` collection.

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

Step with `gs`, trace with `t <n>`, set breakpoints with `b`, display memory with `d`. Registers update live. Type `?` for the full command list.

### Two quirks of the shipped `debug` binary

These are bugs in Microware's own 1980s `debug`, not in os9exec — there is nothing to fix on our side, but they will waste your afternoon if you don't know them.

- **Set breakpoints by name (`b main`) — that works.** Do **not** take an address out of the `sc` symbol listing: `sc` double-counts the module's relocation, printing `real address + symbol offset`. Its addresses are wrong, and wrong by an amount that *grows* the deeper a symbol sits in the module, so the listing looks perfectly plausible. In one test `sc` placed `main` at an address that disassembles as `sprintf+0x22E`. The symbol *names* are fine; only the addresses are corrupt.
- **`gs` is not really a single-step.** It plants a temporary breakpoint at the fall-through address (`PC + instruction length`) and runs to it. So it steps *over* `bsr`/`jsr`, and across a *taken* branch it keeps going until that fall-through is reached anyway — a whole loop iteration later, or never, if the address is unreachable (dead code after a `bra`). Prefer `b <name>` + `g` to land on a chosen spot. A runaway is always recoverable with **Ctrl-C**, which returns you to a fresh `dbg:` prompt.

When in doubt, disassemble an address (`di <addr>`) before trusting it — a function entry should look like a prologue.

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

## Checking a build

One command runs every gate this project has and prints one verdict:

```sh
make verify         # host + Linux (docker)
make verify-vms     # ... and the real virtual machines
make verify-quick   # host only, no containers
```

It exits non-zero if anything failed, so it works as a pre-commit gate rather
than something to read. The individual pieces are still there if you want one
of them on its own — `make test`, `make test-notick`, `make warnings`,
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

The full syscall surface — file I/O, process management, module loading, pipes, events, signals, traps, the shell — runs correctly.

**Time:** os9exec has no internal clock. `F$Time` delegates to the host, so `date` and file timestamps always reflect the host's system time. `setime` accepts a date but has no effect — the host clock is authoritative.

**File permissions:** enforced on **RBF disk images** (including RAM disks, `mount -r` — same filesystem code, just backed by memory instead of a host file). A file/directory is stamped with its creator's `group.user` at creation, and owner/public read-write-execute bits are checked on open, create, delete, and `attr` changes — with an unconditional super-user bypass, matching real OS-9.

Host-native devices (a plain host folder mapped as an OS-9 filesystem, including `OS9DISK` itself) have no on-disk file-descriptor sector to hold an owner byte — ownership isn't tracked there (every file reads as owned `0.0`), and there's no software bypass for super-user the way RBF has one. But `attr`'s read/write/execute bits *are* mapped to a real host permission change, as faithfully as each platform allows:
- **Unix (macOS/Linux):** owner and public map independently to real `chmod` bits (owner → `u`, public → `g`+`o`), the same for read, write, and execute. A file with no read bits genuinely can't be read back — including by `os9exec` itself — until `attr +r`/`+pr` restores it, the same as any other Unix process.
- **Windows:** write is a single, host-wide flag (`FILE_ATTRIBUTE_READONLY`) — there's no separate owner-vs-public write on this platform, so clearing only one side still leaves the file writable. Read denial uses a real NTFS ACL (`DENY` for `Everyone`), which genuinely blocks access — but mingw's own `stat()` doesn't consult ACLs, so `attr`'s *display* won't reflect a read denial even though it's really enforced underneath. Execute isn't real there either way — NTFS has no per-file executable bit, so a host-native file always reads as executable on Windows regardless of its actual state.

If you need OS-9 ownership semantics (not just read/write/execute enforcement) to mean something, use an RBF image (`mount -k=<size>`).

Owner `0.0` is intentional, not unfinished — there's nowhere on a host-native device to store a real one. A file with no public-write bit is only editable as super-user, even by the account that created it (e.g. your own `.login`); add `attr +pw` if you want it self-editable. That same fake `0.0` owner also means `copy`/`dsave` from a host-native source onto an RBF image only carries its attributes over when the copying process is super-user; otherwise the RBF copy keeps its own default attributes and needs `attr` run by hand afterward.

**Hardware-dependent commands** (`backup`, `format`, `tape`, `kermit`, raw `com`, `rdump`, `fsave`/`frestore`) require physical devices that are not emulated and will not work.

**Terminal I/O:** Full screen apps (`vi`, `less`, editors) require `TERM` to be set and a compatible termcap entry. Your disk needs a `SYS/termcap` covering your terminal; the one in the companion `osk-freeware` collection covers `xterm`, `xterm-256color` and `vt100`. An editor is not part of os9exec; `vi`, `sedt` and others are in the companion `osk-freeware` collection.

Everything else in a standard OS-9/68k SDK CMDS directory can be expected to run. See [CMDS.md](CMDS.md) for a command-by-command status list.

---

## Other ways to run it

The [3-step Quick start](#quick-start) is the recommended path — one `make`, and you get every fix above. The [GitHub Releases](https://github.com/peacedudes/os9exec/releases) page still carries only the older tagged `v0.0.0` binaries; no new release has been cut yet.

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
- **Windows x86_64:** native build via [mingw-w64](https://www.mingw-w64.org/) — `make OS=Windows_NT CC=x86_64-w64-mingw32-gcc` (cross-compile from macOS/Linux, or run the same command natively in a Windows shell with mingw-w64 installed). Produces `os9exec.exe`. Docker and WSL2 remain available too.
- **Windows ARM64 (native, not emulated x64):** confirmed working with two common clang-based toolchains, both requiring zero source changes — `make CC=clang` from an [MSYS2](https://www.msys2.org/) `CLANGARM64` shell (`pacman -S mingw-w64-clang-aarch64-toolchain make`), or the standalone [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) distribution (`make CC=aarch64-w64-mingw32-clang`). Visual Studio's `clang-cl`/MSVC toolchain does **not** currently work — it needs a `dirent`/`termios` compatibility layer against Win32 that doesn't exist yet (the codebase had one once, `msdir.c`/`msdir.h`, removed before this branch).
- **Windows 32-bit (x86):** `make OS=Windows_NT CC=i686-w64-mingw32-gcc`. Supported, and built warning-clean on every `make warnings` run — but **never executed**: there is no 32-bit Windows host in this project's test matrix, so unlike every other platform here, nothing has ever run the binary it produces. The 32-bit toolchain does catch things the 64-bit one cannot (it found a pointer-width bug and a `__stdcall` bug the day it was added), so the compile is meaningful evidence — it just isn't the same evidence as a passing test suite. If you run it, we would like to hear either way.
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

### NitrOS-9 (6809) REPL

`tools/nitros9repl.sh` provides the same interface against a *real NitrOS-9
system* running on an emulated CoCo3 — XRoar's "becker port" tunnels
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
[drpitre/drivewire](https://github.com/drpitre/drivewire) `main` — no patched
build is needed, since both halves of the `tcp listen`/`tcp join` protocol
ship upstream. The disk needs `inetd&` in its `startup` and a matching port
line in `SYS/inetd.conf`. The script header documents paths and environment
overrides.

---

## Credits

**OS9exec was created by Lukas Zeller and Beat Forster**, 1993–2007, and is theirs. Everything below stands on their work.

Original project: <http://www.synthesis.ch/os9exec>  
Their final release: **V3.39**, 11 May 2007 — archived at <https://sourceforge.net/projects/os9exec/> (historical; this fork does not publish there)  
This continuation: <https://github.com/peacedudes/os9exec> — Robert Doggett, with Claude (Anthropic)  
License: GNU General Public License v2 or later (see source file headers)

Work in this fork: ports to Apple Silicon, modern Linux and native Windows; verification on riscv64 and on big-endian s390x and sparc64; a 68k conformance suite run against the published manuals; and fixes across the syscall surface, RBF record locking, terminal I/O and the scheduler. Roughly 800 commits on top of V3.39 — see [What's new](#whats-new).

### Reference

*The OS-9 Guru, Book 1: The Facts* — Galactic Industrial Ltd., 1995.  
Landing page: <https://www.icdia.co.uk/books_os9/os9guru/index.html>  
Full scan: <https://archive.org/details/galactic-industrial-the-os-9-guru-1-the-facts>

This book was the primary reference for reconstructing OS-9 kernel struct layouts and system call conventions used in the arm64 port. Galactic Industrial published it specifically to enable third-party OS-9 interoperability work.
