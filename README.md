# OS9exec — arm64 port

OS9exec is an OS-9/68k emulator originally written by Lukas Zeller and
Beat Forster for Classic Mac OS and later ported to Windows and Linux.
This branch (`arm64-uae-integration`) ports it to **macOS on Apple Silicon**
(arm64), replacing the original 68k interpreter with the UAE 68000 engine
and fixing all 64-bit data-width bugs introduced by the LP64 memory model.

Original project: <http://www.synthesis.ch/os9exec>  
License: GNU General Public License v2 (see source file headers)


## What works

A broad set of OS-9 commands run correctly on the arm64 build:

`echo`, `dir`, `pd`, `copy`, `del`, `rename`, `makdir`, `deldir`,
`list`, `count`, `grep`, `merge`, `dump`, `touch`, `cmp`, `attr`,
`ident`, `load`, `unlink`, `mdir`, `procs`, `devs`, `free`, `date`,
`sleep`, `pr`, `tr`, `qsort`, `tee`, `compress`, `expand`, `dsave`,
`fixmod`, `dcheck`, `save`, `printenv`, `help`

Shell features: `>` (stdout redirect), `>>` (stderr redirect),
`<` (stdin redirect), `!` (pipe), `;` (sequential), `&` (concurrent),
named pipes via `/pipe/`.


## Known limitations

- `fsave` / `frestore` — require a tape device (`/mt0`), not emulated
- `math` / `math881` — require a math module not present on typical disk images
- `xmode /term` — requires a terminal driver module
- `rename` — in-directory rename only; cross-directory rename not supported
- `format` — destructive; not tested


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
