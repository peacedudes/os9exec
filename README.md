# OS9exec — arm64 port

OS9exec is an OS-9/68k emulator originally written by Lukas Zeller and
Beat Forster for Classic Mac OS and later ported to Windows and Linux.
This branch (`arm64-uae-integration`) ports it to **macOS on Apple Silicon**
(arm64), replacing the original 68k interpreter with the UAE 68000 engine
and fixing all 64-bit data-width bugs introduced by the LP64 memory model.

Original project: <http://www.synthesis.ch/os9exec>  
License: GNU General Public License v2 (see source file headers)


## Command compatibility

All 88 commands in the `CMDS` directory have been exercised. The table
below summarises findings.

### Works correctly

| Command | Notes |
|---------|-------|
| `attr` | show / set file permissions |
| `binex` / `exbin` | binary ↔ Motorola S-record exchange |
| `build` | create shell scripts (use `< file` redirect to avoid interactive stdin) |
| `cc` / `c68` / `cpp` | C compiler driver and parser — runs; needs `/dd/defs/` include files |
| `cfp` | show disk catalog |
| `cmp` | compare files |
| `compress` / `expand` | Lempel-Ziv compress/expand |
| `copy` | copy files |
| `count` | count lines/words/chars |
| `date` | display current date/time |
| `dcheck` | verify RBF disk structure |
| `deiniz` | detach device (shows usage without args) |
| `del` | delete file |
| `deldir` | delete directory (use `-q` to suppress per-file prompts) |
| `devs` | list open devices |
| `dir` | list directory |
| `dsave` | generate disk-restore shell script |
| `dump` | hex dump |
| `echo` | print arguments |
| `events` | list active OS-9 events |
| `expand` | see `compress` |
| `fact` | arbitrary-precision factorial — runs indefinitely from a starting value |
| `finder` | find files by name |
| `fixmod` | recalculate module CRC / parity |
| `free` | show disk free space |
| `grep` | search for pattern |
| `hasher` | compute file hash |
| `help` | inline help for any command |
| `ident` | display module header |
| `iniz` | attach device (shows usage without args) |
| `irqs` | list IRQ handlers |
| `l68` | linker — runs; reports no input gracefully |
| `link` | link module already in memory |
| `list` | print file contents |
| `load` | load module from disk |
| `login` | attempts chdir to `/h0` home; fails if H0 disk not mounted |
| `mactype` | show/set Macintosh file type |
| `make` | make utility — correctly reports missing makefile |
| `makdir` | create directory |
| `math` | this is a **trap handler module**, not a shell command |
| `mdir` | list loaded modules |
| `menuitem` | insert Macintosh menu item (Ultrascience extension) |
| `merge` | concatenate files |
| `mfree` | show memory free |
| `moded` | OS-9 module editor — interactive, reads from terminal |
| `newmenu` | install Macintosh menu (Ultrascience extension) |
| `o68` | object file tool — reads from stdin when no args given |
| `os9gen` | write OS-9 boot record — requires device arg |
| `pd` | print current directory |
| `pr` | format file for printing |
| `printenv` | print environment variables |
| `procs` | list processes |
| `qsort` | sort stdin |
| `r68` | macro assembler — reports "no input file" without args |
| `rename` | rename file |
| `save` | save module to file |
| `sizeh0` | size an H0 disk image |
| `sleep` | sleep N seconds |
| `tee` | copy stdin to file and stdout |
| `tmode` | show/set terminal mode |
| `touch` | create/update file |
| `tr` | translate characters |
| `tsmon` | timesharing terminal monitor |
| `unlink` | unlink module from memory |
| `xmode` | extended terminal mode display |

### Interactive only (not scriptable)

| Command | Notes |
|---------|-------|
| `break` | drops into the os9exec built-in debugger |
| `code` | interactive hex encoder |
| `debug` | 68k machine-level debugger |
| `edt` | line-oriented text editor |
| `romsplit` | ROM image splitter — reads interactively |
| `testmon` | monitor/terminal hardware test |
| `umacs` | Emacs-like editor — requires `TERM` environment variable |

### Hardware-dependent (not emulated)

| Command | Requires |
|---------|----------|
| `backup` | raw device access (`/dd@`) |
| `com` | serial port |
| `format` | raw writable device |
| `frestore` / `fsave` | tape drive (`/mt0`) |
| `kermit` | serial port |
| `maps` | SSM (Sound/Screen Manager) module |
| `rdump` | raw device |
| `tape` | tape drive |

### Known bugs

| Command | Issue |
|---------|-------|
| `screen` | Crashes in `F$CpyMem` with A0 = 0xFFFFFFF2; the program uses Mac QuickDraw globals (negative A5-relative offsets) which are not present in the emulation |
| `setime` | Accepts only 2-digit years; goes into infinite input loop on bad format |
| `c68` | Reads from stdin when no file args given (hangs in non-interactive use) |

### Shell features

`>` stdout redirect · `>>` stderr redirect (not append — OS-9 convention) ·
`<` stdin redirect · `!` pipe · `;` sequential · `&` concurrent ·
`/pipe/name` named pipes (disappear when drained)


## Requirements

- macOS 14+ on Apple Silicon (arm64)
- Xcode Command Line Tools (`xcode-select --install`)
- A licensed OS-9 disk image (RBF format) mounted as `dd` in the repo root,
  or pointed to by the `OS9DISK` environment variable
- The four adapted OS-9 header files in `Source/OS9exec_core/os9defs/`
  (see below — these are not included due to copyright)


## Build

```sh
make
```

Produces `os9exec` in the repo root. The build uses Apple clang with the
UAE 68000 engine; `-m32` is not used.


## Run

```sh
OS9DISK=/path/to/your/disk.image ./os9exec /dd/CMDS/shell
```

`OS9DISK` is the path to an RBF-format OS-9 disk image. It appears inside
the emulator as `/dd`. `OS9CMDS` and `OS9MDIR` can override the default
command and module search paths.

Send ESC (`\033`) on a line by itself to exit the shell cleanly.


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
