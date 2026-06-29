# OS9exec — macOS arm64

OS9exec is a complete OS-9/68k emulator. You give it a directory of OS-9 binaries,
type `shell`, and you get a real OS-9 command line on your Mac — running the actual
Microware utilities, the actual shell, the actual tools.

It works remarkably well. The shell, pipes, redirection, background jobs, the file
manager, the module system — all solid. If you have a collection of OS-9 software
gathering dust somewhere, this is the fastest way back in.

**No build required.** The `os9exec` binary for macOS arm64 (Apple Silicon) is in
the repo. Clone it, point it at your OS-9 files, and run.


## Three-minute start

```sh
git clone <this-repo>
cd os9exec-git_code

# Put your OS-9 binaries in dd/CMDS
mkdir -p dd/CMDS
cp /path/to/your/os9/CMDS/* dd/CMDS/

# Run the shell
OS9DISK=$(pwd)/dd ./os9exec /dd/CMDS/shell
```

That's it. You're in an OS-9 shell. Type `dir /dd/CMDS` to see what's there,
run anything you like, and press ESC on a blank line to exit.

> **macOS security note:** The first time you run a downloaded binary, macOS may
> block it. Open System Settings → Privacy & Security, scroll down, and click
> Allow. Or: `xattr -d com.apple.quarantine os9exec`


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

The reconstruction used *The OS-9 Guru* (Galactic Industrial) as the primary source:
OS-9 struct layouts were derived from the book's field descriptions and built into
four new header files (`procid_from_book.h`, `module_from_book.h`, etc.) committed
to this repo. The F$DFork handshake — specifically that A2 holds the parent's
register buffer address, stored by the kernel into `P$DbgReg` of the child's process
descriptor — was confirmed from a fragment of the real kernel's `fork.a` assembly.
The fix was then a small, precise change: capture `rp->a[2]` at DFork time, write
child registers there in the correct 72-byte R$ frame layout, and inject `P$DbgPar`
as a non-zero sentinel so the debugger binary recognises the child as being debugged.

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
| `/h0`–`/h9`, `/ha`–`/hz` | `OS9H0=…` through `OS9HZ=…`, or dirs named `h0`–`hz` next to the binary | Additional host directories |

Files placed in a host directory appear immediately inside the emulator as OS-9
files, with no conversion needed for binary modules. Text files need OS-9 line
endings (CR, `0x0D`) rather than Unix LF — the emulator handles this transparently
for `I$ReadLn`/`I$WritLn`, but raw byte copies preserve whatever endings are in
the file.


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
| `-h` | Full option list |


## Built-in commands

When launched normally, os9exec intercepts a set of names before the OS-9 shell
can fail on them. They run inside the emulator and look like regular OS-9 programs:

| Command | What it does |
|---------|--------------|
| `ihelp` / `icmds` | List all built-in commands |
| `iprocs` | Show running OS-9 processes |
| `imdir` | Show loaded OS-9 modules |
| `ipaths` | Show open paths |
| `imem` | Show memory blocks |
| `idevs` | Show mounted devices |
| `idbg` / `debughalt` | Enter the emulator's interactive debugger |
| `stop` / `shutdown` | Exit os9exec cleanly |
| `rename` | Rename a file or directory |
| `move` | Move files or directories |
| `ls` | Extended directory listing |
| `mount` / `unmount` | Mount or unmount an RBF image at runtime |

Pass `-i` to disable all of these and use only real OS-9 binaries.


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
events, signals, traps, the shell — runs correctly. A few things are not emulated:

| Item | Status |
|------|--------|
| `screen` | Crashes — uses Mac QuickDraw globals at negative A5-relative offsets |
| `setime` | Loops on bad date format; accepts only 2-digit years |
| `c68` | Reads stdin when no file argument given — hangs in non-interactive use |
| Raw device commands (`backup`, `format`, `tape`, `kermit`, …) | Require physical hardware not emulated |


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
