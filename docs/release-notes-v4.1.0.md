# os9exec v4.1.0

The first update since v4.0.0. It makes os9exec a good deal more faithful to the OS-9/68000
manuals, lets OS-9 networking programs reach the real network, adds a browser build, and fixes
a long list of things found by running a large collection of real OS-9 software under it.

Nearly every fix listed here has a test that fails on the build before it and passes on this
one. The suite runs on macOS and on Linux (x86-64, i386 and big-endian s390x), and the
conformance suite (below) on every one of them.

## Highlights

**Networking.** OS-9 programs that use sockets now reach the host's network. Microware's own
networking programs work: `telnet`, `ftp`, `ping`, `tcpsend`/`tcprecv`, `tftpd`, `msend`/`mrecv`,
RPC (`rsort` through `portmap`), and the `telnetd` and `ftpd` servers, which host clients can
log in to. Programs built on the older socket library that opens `/socket`, such as `ttcp` and
the BIND 4.8.3 tools `nslookup` and `nsquery`, work too, and so do servers that poll their
listening socket for a waiting connection. Several programs need Microware's `inetdb` and
`netdb` modules loaded first (`/dd/CMDS/BOOTOBJS/SPF`). A socket that is waiting never stops
the rest of the system: other processes keep running. See the README's Networking section.

**A browser build.** os9exec now also builds to WebAssembly and runs in a web page with a
terminal. The page can keep a disk of your own in the browser's storage as `/h1`, and saves it
when the page is hidden or closed. A `?run=` link types a command for you and leaves pressing
Enter to you, so a link can never run anything on its own. It has no network and no host
directories. See the README.

**Idle is idle.** An os9exec whose processes are all waiting now sleeps until something is due
instead of polling: about 0.1% of a core, down from about 1.6%. A waiting reader or writer
resumes when its data arrives.

**Built-in commands.** New: `pwd` and `cd`, and `iterm`, which makes a `/tN` terminal at runtime
the way `mount` makes an `/hX` disk. `devs` now lists host directories as well, says who is using
each device and how to attach to a terminal, and fits 80 columns. `mount <image> hX` now attaches
an existing disk image while running.

## Changes you might notice

A few fixes change behaviour that a script could have come to rely on:

- Deleting a file that is open, anywhere in the emulator, is refused with `E$Share` on a host
  directory as well as on an RBF image. Close it first.
- The 68881 now rounds a float stored as an integer the way the FPU is set, to nearest by
  default: 2.7 stored as a long is 3, where it used to be truncated to 2. Out-of-range values
  saturate instead of wrapping. BASIC09's own `INT`, `FIX` and integer assignment are unchanged.
- `F$Link` and `F$UnLoad` with a type or language look only for a module of that kind.
- The browser page's `?run=` link no longer presses Enter.

## Compatibility

System calls, now as the Technical Manual describes them:

- Events: `Ev$Wait` returns the value that satisfied it, not the value after its increment
  (period code that uses an event as a lock hung on this); an empty event name is refused.
- Modules: `F$Link` refuses a module the caller has no read permission for; sticky modules stay
  in memory at link count 0; a higher revision supersedes the resident module, and of equal
  revisions the established one keeps answering; a same-named module of another type no longer
  hides the one asked for; `F$Load` links only a file's first module, and the module directory
  names that module as the group; `F$DatMod` makes a Data module unless asked for another type,
  and records its creator.
- Memory: `F$Mem` resizes the data area in place, and what it adds is always clean;
  `F$SRqMem` with -1 allocates the largest free block; a request near 4 GB is `E$NoRAM` on every
  host; a process may hold 8192 blocks; `F$GBlkMp` reports the free map.
- Processes and signals: `F$STrap` handlers get the PC in a0 and run on their own stack, for
  every vector the manual lists; signals queued while masked reach the intercept routine in
  order, with the count in d0; the mask level stops at 255; a process's own `S$Wake` wakes it;
  `F$DExec` single-steps system calls and resumes from the debugger's register buffer.
- CPU time: a process that computes without making system calls is charged for it, so the C
  library's `clock()` advances (the Whetstone benchmark used to divide by zero).
- Permissions: `S$Kill`, `F$SPrior`, `F$SetSys` and `F$DExit` enforce the manual's rules.
- Time: `F$Alarm` IDs, cycles and the 256ths-of-a-second interval form; an absolute alarm already
  past is sent; `F$Sleep` rounds up; `F$Julian` and `F$Gregor` use the Julian calendar before
  1582 and get the century leap years right.
- New: `F$SchBit`, `F$AllBit`, `F$DelBit`, `F$SysID` (pre-3.0 form), and much more of `F$SetSys`.
- The 68000 core: `MOVE from SR` is user-legal; `NEG`, `NBCD` and `SUB` set X; the 68881
  emulation stores doubles exactly (results were sometimes one bit off) and reports infinities,
  so a division by zero under `math881` is an error rather than a wrong number. `FINT` rounds to
  nearest even, and `FINTRZ`, `FMOD` and `FREM` are exact beyond 32-bit values.

Files and devices:

- RBF disk images: record and end-of-file locking now follow the manual, including a reader
  following a writer and a writer that moves away from the end; two paths writing one sector
  no longer lose each other's bytes; a file that another path has open cannot be deleted;
  directory permissions, the single-user bit, `SS_Size`, `SS_Attr`, `SS_FD` and `SS_Ticks`
  behave as documented; relative `../..` works; 28-character names work.
- RBF integrity: a pre-release review found, and this release fixes, several ways an image could
  be damaged: a byte patched after a multi-sector write reverting it; a file trimmed at close
  while another path still used it; a hard-linked file's sectors freed by deleting one of its
  names; allocations of 65536 sectors or more half recorded; a directory held open writing a
  stale sector over a new entry; and a disk filled part way by `SS_Size` losing the sectors it
  took. An image claiming a sector size os9exec cannot read is refused.
- Host directories: files can be renamed and moved by rewriting their entry, as OS-9's `move`
  does; directory entries keep their positions across a deletion; a symlink cannot lead out of
  the device; paths with spaces work, and so do host names longer than 28 characters in the
  middle of a path; lookups are much faster (a device scan that took minutes on a Docker share
  takes seconds).
- SCF and pipes: console line editing honours the path options (end-of-record, padding, the
  512-byte line buffer); the status codes the manuals give SCF and pipes are implemented; a
  signal ends a blocked pipe read; two writers' lines no longer interleave on one terminal.
- `-d` tracing and the debugger talk to you, never into the program's own output.
- 32-bit hosts (Linux i386, 32-bit Windows, the browser): several sizes that wrapped there, and
  a module file that could hang `F$Load`, are fixed.

## If you have real OS-9/68000 hardware

os9exec is built from the manuals, and where the manuals leave room it has had to make a
reading. The conformance suite, CONF68K, turns those readings into tests with the manual's
words beside each, and it runs on real OS-9 as well as here. This release attaches it as
a single disk image, `conf68k.dsk`: 109 standalone tests, hand-written assembly, no Microware
software on it. Put it on a disk device, `chd` to it and type `runall`; each test prints one
line with what it observed and what the manual led us to expect. Where a claim is our inference
rather than the manual's plain statement, the claim says so. If your hardware disagrees with
us anywhere, please open an issue with the RESULT lines: that is the one thing an emulator
cannot tell us for itself.

## Known limitations

- A file manager or device driver supplied as an OS-9 module is loaded but never used: the file
  managers are part of os9exec.
- WN's `inetd` needs an `inetdb` module in the older Internet Support Package layout, which is
  not included.
- There are no device descriptor modules in memory for the built-in devices, so tools that link
  a descriptor by name (`xmode /term`, `dmode`) report it missing. Loading one from
  `/dd/CMDS/BOOTOBJS` works around it.
- The browser build has no networking and no host directories.

## Downloads

`os9exec-macos-arm64`, `os9exec-linux-x64`, `os9exec-linux-i386` and `os9exec-windows-x64.exe`,
and `conf68k.dsk` for real hardware. os9exec needs an OS-9/68000 system disk, named with
`OS9DISK`; see the README.

## Thanks

To Lukas Zeller and Beat Forster, who wrote os9exec and are glad to see it running on today's
machines; and to everyone keeping OS-9 software alive, whose programs found most of what this
release fixes.
