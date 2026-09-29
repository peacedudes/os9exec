# os9exec v4.1.0

The first update since v4.0.0. It makes os9exec a good deal more faithful to the OS-9/68000
manuals, lets OS-9 networking programs reach the real network, adds a browser build, and fixes
a long list of things found by running a large collection of real OS-9 software under it:
the [OS-9/68000 freeware collection](https://github.com/peacedudes/osk-freeware), three decades
of community software on one disk image. Most of what is fixed here was found by running it.
The [OS-9 development skills](https://github.com/peacedudes/os9-dev-skill), two
reference collections for OS-9 on the 6809 and 68000 that a person or an AI assistant can
use, grew up alongside: much of what they say was checked by running it under os9exec.

**See it running first:** [open a live OS-9 system in your browser](https://peacedudes.github.io/osk-freeware/try/),
this release compiled to WebAssembly and booted from the freeware collection's disk.

Nearly every fix listed here has a test that fails on the build before it and passes on this
one. The suite runs on macOS and on Linux (x86-64, i386 and big-endian s390x), and the
conformance suite (below) on every one of them, on Windows 11 and in WebAssembly.

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
Enter to you, so a link can never run anything on its own. It signs on with the original
authors' names whatever program it starts, or with one line (`-l`) where a page embeds a small
terminal. It has no network and no host directories. See the README.

**Idle is idle.** An os9exec whose processes are all waiting now sleeps until something is due
instead of polling: about 0.1% of a core on macOS and Linux, down from about 1.6% (on Windows
about 1%, down from 2 to 4%). A waiting reader or writer resumes when its data arrives, and a
writer waiting on a full pipe no longer keeps a core busy (in v4.0.0 it spun until its reader
caught up).

**Windows and the browser catch up.** The system tick now runs on Windows and in the browser
too, where there is no timer signal to drive it, so a program that makes no system calls no
longer keeps the machine to itself: other processes run, Ctrl-C and Ctrl-E reach it (checked
by hand on Windows 11), and the browser page keeps drawing and taking keys. On Windows, a
program's standard input redirected from a file or a pipe is read to its end (a file gave
nothing, and a pipe never ended), and a Windows text file's CR LF is one line end rather than a
line end and a stray LF. Output redirected to a file ends its lines CR LF as on macOS and Linux
instead of CR CR LF, and an idle console wakes on a keystroke.

**A check for stray writes.** `-W` makes a program's write outside its own memory (its data
area, the blocks it requested, loaded modules) a bus error, and names the address and the
instruction, as OS-9's SSM does on real hardware. It is off by default. It is a debugging aid:
our own test programs once wrote 32K past their data for months, and it showed only as a
utility looping in one session layout.

**Built-in commands.** They are neither files nor modules, so `dir` and `mdir` cannot show them;
the sign-on now ends by pointing at `ihelp`, which lists them, and every one answers `-?`
(`stop -?` used to stop). New: `pwd` and `cd`, and `iterm`, which makes a `/tN` terminal at
runtime the way `mount` makes an `/hX` disk. `devs` now lists host directories as well, says who is using
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
- `F$Fork` and `F$Load` refuse a module the caller has no read permission for (`E$Permit`), as
  `F$Link` does: a program readable only by its owner no longer runs for other users. A module
  that is not re-entrant is linked by one process at a time; another process's `F$Link`, `F$Load`
  or `F$Fork` of it gets `E$ModBsy`. No program on the freeware disk is affected: all of its
  modules are re-entrant.
- Page pause (`tmode pause`) now pauses: output stops after each page until a key is pressed,
  and that key is not passed on to the program. It used to stream on and eat the next key typed.
- A host directory refuses to create a name longer than 28 characters (`E$BPNam`).
- The browser page's `?run=` link no longer presses Enter.
- The CPU is reported as a 68020, which is what is emulated (with a 68881) and what `F$SysID`
  already said. `D_MPUTyp` and the `init` module said 68040.

## Compatibility

System calls, now as the Technical Manual describes them:

- Events: `Ev$Wait` returns the value that satisfied it, not the value after its increment
  (period code that uses an event as a lock hung on this); an empty event name is refused.
- Modules: `F$Link`, `F$Load` and `F$Fork` refuse a module the caller cannot read; sticky modules stay
  in memory at link count 0; a higher revision supersedes the resident module, and of equal
  revisions the established one keeps answering; a same-named module of another type no longer
  hides the one asked for; `F$Load` links only a file's first module, and the module directory
  names that module as the group; `F$DatMod` makes a Data module unless asked for another type,
  and records its creator.
- Memory: `F$Mem` resizes the data area in place, and what it adds is always clean;
  `F$SRqMem` with -1 allocates the largest free block; a request near 4 GB is `E$NoRAM` on every
  host, and one near 2 GB no longer clears memory it was never given on a 32-bit host;
  a process may hold 8192 blocks; `F$GBlkMp` reports the free map; a zero-byte block can be given
  back without crashing the emulator.
- Processes and signals: `F$STrap` handlers get the PC in a0 and run on their own stack, for
  every vector the manual lists; signals queued while masked reach the intercept routine in
  order, with the count in d0; the mask level stops at 255; a process's own `S$Wake` wakes it;
  `F$DExec` single-steps system calls, resumes from the debugger's register buffer, and steps
  only its own child (a program running beside a debugged one that waited at the terminal was
  stepped too, and crawled). A signal handler that waits on a pipe no longer freezes the whole
  emulator (it did in v4.0.0), and `F$RTE` goes back to the request the signal interrupted,
  with the condition codes it found. A program computing without system calls gets its signal
  at the next tick, and a second signal waits its turn instead of being lost.
- `F$Fork` and `F$Chain` run only program object code, and refuse anything else with `E$NEMod`
  (a packed BASIC09 module was "bad module ID"). `os9exec <module>` does what a shell does and
  hands a packed BASIC09 module to RunB. A refused fork gives back its data area and its module
  link, and a failed `F$Chain` no longer unlinks its module twice.
- The system tick runs even when os9exec is started with SIGALRM blocked, as some launchers
  (GitHub's macOS runner among them) start programs; it used to be silently absent there.
- CPU time: a process that computes without making system calls is charged for it, so the C
  library's `clock()` advances (the Whetstone benchmark used to divide by zero).
- Permissions: `S$Kill`, `F$SPrior`, `F$SetSys` and `F$DExit` enforce the manual's rules.
- Time: `F$Sleep` rounds up, and ends on time beside a process that computes without system
  calls (it overran by up to 30 ticks, and by seconds on a host with a slow timer); `F$Alarm`
  IDs, cycles and the 256ths-of-a-second interval form work; an absolute alarm already past is
  sent; `F$Julian` and `F$Gregor` use the Julian calendar before 1582 and get the century leap
  years right.
- New: `F$SchBit`, `F$AllBit`, `F$DelBit`, `F$SysID` (pre-3.0 form), and much more of `F$SetSys`.
- The 68000 core: `MOVE from SR` is user-legal; `NEG`, `NBCD` and `SUB` set X; the 68881
  emulation stores doubles exactly (results were sometimes one bit off) and reports infinities,
  so a division by zero under `math881` is an error rather than a wrong number. Arithmetic and
  `FINT` round as the FPU's rounding mode says (to nearest even by default; the browser build
  always rounds to nearest, as WebAssembly has no other mode), and `FINTRZ`, `FMOD` and `FREM` are
  exact beyond 32-bit values. Also per Motorola's manual: `FSCALE` is exact; `FGETEXP` and
  `FGETMAN` answer zero and infinity correctly; `FMOD` and `FREM` set the quotient byte, which
  other instructions no longer clear; the not-greater-or-equal condition is right; a byte
  immediate operand reads the right byte; `FScc (An)+` moves An; packed-decimal infinities and
  NaNs convert. A bus-error handler sees the program's condition flags as they were.

Files and devices:

- RBF disk images: record and end-of-file locking now follow the manual, including a reader
  following a writer and a writer that moves away from the end, and `SS_Lock` from a path in
  any mode (nethack3 locks its log through a write-only path); two paths writing one sector
  no longer lose each other's bytes; a file that another path has open cannot be deleted;
  directory permissions, the single-user bit, `SS_Size`, `SS_Attr`, `SS_FD` and `SS_Ticks`
  behave as documented; relative `../..` works; 28-character names work.
- RBF integrity: a pre-release review found, and this release fixes, several ways an image could
  be damaged: a byte patched after a multi-sector write reverting it; a file trimmed at close
  while another path still used it; a hard-linked file's sectors freed by deleting one of its
  names; allocations of 65536 sectors or more half recorded; a directory held open writing a
  stale sector over a new entry; and a disk filled part way by `SS_Size` losing the sectors it
  took; a write that runs out of disk shortening the file; a file whose segment list fills
  keeping sectors it had just given back; and a directory whose only entry was still unwritten
  counting as empty, so it could be deleted. A file just created reports its own attributes. An
  image claiming a sector size os9exec cannot read is refused, and a `chd` deeper than a path
  can record is refused with `E$BPNam`.
- Host directories: files can be renamed and moved by rewriting their entry, as OS-9's `move`
  does; directory entries keep their positions across a deletion; a symlink cannot lead out of
  the device; paths with spaces work, and so do host names longer than 28 characters in the
  middle of a path; lookups are much faster (a device scan that took minutes on a Docker share
  takes seconds). The internal `rename` can no longer rename a device's own root directory, and
  open paths and current directories follow a rename; a listing no longer ends early once many
  other directories have been read; a raw path to a device no longer blocks deleting a file, and
  `SS_Attr` and `SS_FD` through one are refused rather than reaching the host directory.
- A pathlist on a device that does not exist is `E$MNF` for every call, as OS-9 reports it when
  it cannot link the device's descriptor; `chd`, `del` and `makdir` said `E$BPNam` where `dir`
  said `E$MNF`.
- `-6` also opens RBF disks without the OS-9/68000 format's "Cruz" mark, as 6809 (CoCo) disks
  are: the file system is the same RBF, and listing, reading and `dcheck` work. It is off by
  default, because that mark is what tells a 68000 disk from a 6809 one, so reading an unmarked
  disk is something you ask for rather than something os9exec assumes.
- SCF and pipes: console line editing honours the path options (end-of-record, padding, the
  512-byte line buffer); the status codes the manuals give SCF and pipes are implemented; a
  signal ends a blocked pipe read; two writers' lines no longer interleave on one terminal.
  `SS_Size` on a pipe is refused rather than answered with the buffer's size, so `less` pages
  piped input to the end instead of stopping after 4K. An echo waiting behind `^S` no longer
  keeps a core busy; a signal ends a write parked on a terminal with the signal as its error;
  `I$WritLn` to a terminal ends the record at its own end-of-record character. A built-in
  command held by `^S` that nobody lifts waits 10 seconds once and then writes, in order; it
  used to stop the whole emulator for two minutes per character. After `F$RTE`, a write resumes
  behind one that began while its intercept routine ran, instead of inside it.
- Keyboard: `^C`, `^E` and XON act even behind a full type-ahead buffer (typed or pasted text
  beyond it waits rather than being lost), and while a program computes without system calls,
  where before the keys went nowhere and it could not be stopped from the keyboard. A program
  with an intercept routine gets the key (BASIC09 stops at `BREAK` in a loop); from the shell,
  `^E` ends one without, and `^C` sends one that has written nothing to the background, as the
  manual says.
- `/tN` terminals: one bound to a host serial port takes its parity, bits per character and stop
  bits from the path (`tmode par= cs= stop=`), as it already took its speed; one that takes part
  of a line ending gets the rest once, not twice; a write to one whose far end has gone ends with
  `E$Write` instead of waiting for ever.
- Sockets: a send cut short by a signal no longer makes the next send from the same buffer skip
  bytes.
- `-d` tracing and the debugger talk to you, never into the program's own output. The debugger's
  `k` refuses a process that does not exist, `n` keeps the whole trigger name, a listing taken
  while tracing no longer moves the program's own PC, and its process, module, file and memory
  listings reach you even when `idbg`'s output is redirected. Under `-d`, a traced pipeline's
  second program no longer waits for the first to finish before its output appears.
- 32-bit hosts (Linux i386, 32-bit Windows, the browser): several sizes that wrapped there, and
  a module file that could hang `F$Load`, are fixed; so is a clock that froze paced terminal
  output for over an hour every 72 minutes, and the size of a disk image of 2 GB or more.

## If you have real OS-9/68000 hardware

os9exec is built from the manuals, and where the manuals leave room it has had to make a
reading. The conformance suite, CONF68K, turns those readings into tests with the manual's
words beside each, and it runs on real OS-9 as well as here. This release attaches it as
a single disk image, `conf68k.dsk`: 116 standalone tests, hand-written assembly, no Microware
software on it. Put it on a disk device and, as the super user, `chd` to it, `chx CMDS`, and
type `runall >>+RESULTS/errors`. At the end it says in plain words whether there is anything to
send: only a test that fails is news, and then it asks you to open an issue with
RESULTS/report. A SKIP is not a failure; it means that claim cannot be checked on your system.
Where a claim is our inference rather than the manual's plain statement, the claim says so.

The same disk runs under os9exec with no OS-9 system disk at all, on any platform:
`OS9DISK=/path/to/conf68k.dsk os9exec -r /dd/CMDS/run`. There every test passes (the one
SKIP is `load`, which is Microware's and not on the disk).

## Known limitations

- A file manager or device driver supplied as an OS-9 module is loaded but never used: the file
  managers are part of os9exec.
- WN's `inetd` needs an `inetdb` module in the older Internet Support Package layout, which is
  not included.
- There are no device descriptor modules in memory for the built-in devices, so tools that link
  a descriptor by name (`xmode /term`, `dmode`) report it missing. Loading one from
  `/dd/CMDS/BOOTOBJS` works around it.
- The browser build has no networking and no host directories.
- The Windows build has no networking (socket paths answer `E$Unit`) and no host terminals
  for `/tN`.

## Downloads

`os9exec-macos-arm64`, `os9exec-linux-x64`, `os9exec-linux-i386` and `os9exec-windows-x64.exe`,
and `conf68k.dsk` for real hardware. os9exec needs an OS-9/68000 system disk, named with
`OS9DISK`; see the README.

## Thanks

To Lukas Zeller and Beat Forster, who wrote os9exec and are glad to see it running on today's
machines; and to everyone keeping OS-9 software alive, whose programs found most of what this
release fixes.
