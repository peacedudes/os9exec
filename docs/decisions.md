# Design decisions

This file records why os9exec is the way it is. It covers the choices that a
reader of the code, or the author of a pull request, would otherwise have to
re-derive: what was decided, what was declined or deliberately left alone, and
what was reversed later. A declined idea that leaves no trace tends to be
proposed again, and the next person to think of it has to rebuild the
reasoning from scratch. Each entry gives the decision, the reasons that carried
weight, and the cost that was knowingly accepted.

os9exec follows the Microware manuals (chiefly the OS-9/68000 v2.4 Technical
Manual, the Technical I/O Manual and *Using Professional OS-9*). Where it
departs from them, or where the manuals are silent and a choice had to be
made, this file says why. Where a choice is only an inference, the CONF68K
conformance suite (`test/68k-conformance`) carries a test marked INFERENCE, so
that a run on real OS-9/68k hardware can settle it; `DOCS/claims.md` there
gives each test's citation.

Outcomes used in the headings:

- **Declined**: considered and not done.
- **Kept as is**: a known difference from the manual's words, kept on purpose.
- **Done differently**: done, but not the way first proposed.
- **Reversed later**: an earlier decision that was overturned. Kept here so the
  old idea is not re-proposed.
- **Left open**: not done, not ruled out.

Last updated for v4.1.0 (September 2026).

---

## Filesystems: RBF images and host directories

os9exec mounts two kinds of storage: RBF disk images, which it reads and
writes as real OS-9 does, and host directories, which it presents as OS-9
directories. RBF is where fidelity is measured. Host directories are a
convenience bridge with no counterpart on real OS-9;
[`host-drives.md`](host-drives.md) lists how they differ.

### Host directories have no record locking -- Declined

RBF implements the full record-locking mechanism. Host directories have none:
no lock state, no sleeping on a locked record, no host `flock`. An explicit
`SS_Lock` on a host file fails honestly with `E$UnkSvc`; on a host directory
it is accepted and ignored. What is absent is the automatic read-lock and
write-release, which is silent because nobody asked for it.

Reasons, in order: there is no Microware behaviour to be faithful to on a
filesystem OS-9 never had; host directories already lose attributes and
ownership; a lock there is only half-true, since host tools can change the
file behind the emulator, whereas an RBF image is opaque to the host; and
`fileaccess.c` handles every operation on a host `/dd`, so adding blocking
there risks a wedged emulator.

**Cost accepted:** concurrent read-modify-write on a host directory can
silently lose updates. The README says so, and says to use an RBF image.
Revisiting this would first need each path's separate host `FILE*` made
coherent with the others.

### Deleting a file that is open -- Reversed later: now refused with E$Share on host directories too

I$Delete (Technical Manual, page 2-7) says the file "may not already be
open". RBF has always enforced this (`0ee76c1`), because allowing it would
leak every cluster a still-open writer allocates afterwards.

This was first declined for host directories, on the grounds that the RBF
reason (allocation bitmap integrity) does not apply and POSIX defines
unlink-while-open. That was overturned in September 2026 once the user-visible
harm was set out: on a Unix host, everything written after the unlink is
silently lost, a state real OS-9 cannot reach. `pFdelete` in `fileaccess.c`
now scans the emulator's own open paths and answers `E$Share` if one holds the
file, comparing resolved host paths so a symlink is still recognised. A miss
(for example under MinGW, which has no `realpath`) falls back to allowing the
delete, the safe direction for a check that cannot be exhaustive. CONF68K t99.

### `del` of an empty directory succeeds on a host directory -- Kept as is

On an RBF image os9exec refuses to delete a directory with `E$FNA`, as
*Using Professional OS-9* describes. On a host directory an empty directory is
removed; a non-empty one is refused with `E$DNE`. Kept because the host
directory is already the lossy layer, the exposure is one empty directory, and
`deldir` must be able to remove directories there, so a refusal would need
care for little benefit.

### RBF sector size is capped at 2048 bytes -- Declined raising it

The v2.4 Technical Manual allows sector sizes up to 32768; os9exec refuses
anything above 2048 (`MIN_TMP_SCT_SIZE`). The total sector count `DD_TOT` is
three bytes, so a device holds at most 16,777,215 sectors whatever their size:
2048-byte sectors already address 32 GiB, far past any media OS-9/68k ran on.
Raising the cap would buy capacity nobody has and would mean sizing every
path's buffers from the device's sector size.

**Cost accepted:** a documented limit of 2048 where the manual says 32768. An
unusable size now reports `E$SectSize` rather than `E$NotRdy` (`52fe70d`);
`CheckSectorSize` in `file_rbf.c` states the reason. Crafted images with
larger sector sizes were checked under AddressSanitizer and are rejected by
RBF's consistency checks before any oversized read, but the fixed-size buffer
in `GetBuffers` is one more reason not to raise the cap casually.

### An image without the OS-9/68000 "Cruz" mark opens only with `-6` -- Decided

The OS-9/68000 RBF format carries the identifier "Cruz" at offset $60 of
sector 0; 6809 disks do not. The mark appears to be there so a 6809 disk
cannot be taken for a 68000 one, and os9exec respects it: by default an
unmarked image is refused, exactly as in v4.0.0. `-6` opens unmarked images
as well (6809 and CoCo disks among them). The file system is the same RBF, so
listing, reading and `dcheck` work. Reading an unmarked disk is something the
user asks for, never something assumed.

### A device that does not exist is E$MNF from every call -- Done

OS-9 fails any call on a device it has no descriptor for when it tries to link
the descriptor, so the answer is `E$MNF`. os9exec's I$Open said so, but
I$ChgDir, I$Delete and I$MakDir answered `E$BPNam` (`eead87b`). And when
os9exec was started inside another device's folder, the fallback for an
unknown two-letter device landed inside that device, so every call said
`E$PNNF` (`4d60001`, found by the Linux test container). A two-letter device
that is neither an image nor a host folder is now no device at all.

### A super user's create on a host directory keeps owner read -- Decided

A host directory's only store for attributes is the host's mode bits, so an
I$Create with attributes `$0000` made a file nobody could read back. On an
RBF image the super user "may access any file" (Technical Manual, page 7-11)
and the reopen works. Period tools passed 0 by mistake and real OS-9, where
they ran as super user, hid it. So on a host directory a super user's create
gets the owner-read bit added, never write: the super user may read any file,
but a file created without write permission stays protected against an
accidental overwrite. RBF images are untouched, as are non-super users and
explicit `attr` changes.

### PD_FD and PD_DFD are byte positions, not sector numbers -- Kept as is

The Technical I/O Manual's RBF path descriptor says PD_FD holds the LSN of the
file's descriptor. os9exec writes a byte position (the LSN times the sector
size on RBF). Kept because the one real consumer we can test agrees with
bytes: GNU bash, built for real OS-9, finds its working directory by comparing
PD_FD of `.` with the entries of `..`, and works on both RBF and host
directories only with this encoding. The live-verification case `getwd-dirfd`
pins it. A real-hardware reading of PD_FD for a file whose FD sector is known
would settle it; if it reads back the LSN, the emulator and the C library's
getwd have to change together.

### SS_Size on an RBF path opened read-only is accepted -- Kept as is

Neither the Technical Manual's I$SetStt SS_Size entry nor the Technical I/O
Manual's RBF SetStat list names an access mode the path must have, so nothing
refuses it. CONF68K t110 reports what the system does, marked as an inference:
`E$BMode` from real hardware would mean os9exec should refuse.

### The EOF lock survives a seek -- Kept, as the manual words it

The Record Locking chapter says the end of file stays locked "until a read or
write is performed that is not at the end". A write away from the end now
releases it (`b447586`); a seek alone does not, since it is neither. CONF68K
t105 pins this and is marked as an inference.

### A raw device open of a host directory is E$Unit -- Kept; no synthesized sector 0

Opening `/hX@` on a host directory is refused with `E$Unit` (`2ceec83`), and
`free`/`dcheck` on one get `E$FNA` (`063f8d1`). Synthesizing an
identification sector was proposed so that a host directory could stand in
for a disk, and declined: `free` and `dcheck` would then report invented
geometry for a directory that has no sectors. A disk that needs to look like a
disk ships as an RBF image instead, which is how the companion freeware disk
is distributed.

### Host I$MakDir creates the host directory as mode 0700 -- Kept as is

Whatever mode bits the caller passes, the host directory is created owner-only
(the RBF half honours them, `1d8fc2d`). This is invisible from OS-9, because
os9exec reports every host directory with all permissions. Revisit only if
that report changes.

### SS_FD on a host directory writes only the date -- Kept as is

A host directory has no file descriptor sector. The fields that do have a host
equivalent are already written by the calls that own them (SS_Attr sets the
mode bits), so SS_FD writes the modification date and nothing else. Stated at
the site in `fileaccess.c`.

### A lone `.AppleDouble` blocks removal of a host directory -- Left as is

`RemoveAppledouble` was meant to clear a netatalk `.AppleDouble` directory
before a host directory is removed, but directory listings never show that
entry, so its removal loop never runs and such a directory refuses deletion
with `E$DNE`. Not fixed: netatalk shares are not a supported setup, and making
the loop live would have os9exec delete host files OS-9 cannot see.

### `pFseek`'s file-extend branch is dead code -- Left as is

The branch in `pFseek` that was meant to extend a host file past its end was
never implemented (the host ioctl it wanted was unavailable), and its
platform conditional returns the same error on both arms. It is not reachable:
POSIX `fseek` past the end succeeds, and pipes bypass `pFseek` entirely. The
behaviour it would have provided is already pinned by CONF68K t07 (seek to
70000, write, size is 70005) and t08 (seek past EOF then read gives `E$EOF`).
It is a latent trap: finishing the extend would silently make the dead
conditional live and give Windows the wrong platform's semantics.

### `mount -k` with the working directory at `/` -- Left open

With the emulator started in the host's root directory, an image whose host
path is `/h9` is textually identical to the OS-9 device name `/h9`, and the
device never registers. It is a namespace collision rather than a path bug.
The container images avoid it by working in `/work`. Status as of v4.1.0:
unverified whether this still reproduces.

---

## Terminals, SCF and pipes

### Only SCF paths honour PD_EOR -- Kept: RBF, pipes and sockets use CR

SCF honours the path's end-of-record character. Elsewhere the literal carriage
return in the I/O layer is correct and stays:

- **RBF and host files.** The RBF path descriptor option section has no PD_EOR
  field, and the RBF chapter names the terminator as the carriage return.
- **Pipes.** Pipeman's option section has no PD_EOR, and the manual says
  pipeman checks for carriage returns on ReadLn and WritLn. A `/ttyNN` pipe
  path is the exception, because it carries a real SCF option table.
- **Sockets.** Their option section is the device type and nothing else.

A reader grepping for `CR` in the I/O layer will find these and wonder; this
entry is the answer.

### SCF edit-buffer details: PD_EOF, PD_NUL and the 512-byte buffer -- Done

PD_EOF ends a read only when the edit buffer is empty, which satisfies both
Microware ("as the first character of the read") and *The OS-9 Guru*'s reading
(first in the buffer, so a line typed and then erased counts again). PD_NUL
padding after a carriage return is implemented; its default is zero, so an
unconfigured path is unchanged. The 512-byte buffer holds 511 data bytes plus
the terminator. A consequence worth knowing: changing PD_EOR also changes what
ends the test harness's own input, so tests that change it restore `eor=0D`
before finishing, as a real user would have to.

### Page pause counts carriage returns, not PD_EOR -- Kept as is

`ConsoleOut` counts screen lines on CR. Microware's PD_PAU description names
no character; *The OS-9 Guru* says explicitly that SCF counts carriage returns
and warns that page pause must be off before sending binary data. The Guru is
not Microware, but it is the only explicit statement and it agrees with the
code. Changing it on no evidence would invent a divergence.

### os9exec does not translate terminal control codes on output -- Declined

`robots` emits a bare `^K` (0x0B) for cursor-up, which every host terminal
reads as a vertical tab, moving the cursor down; the screen scrolls and later
writes land on shifted content. Translating 0x0B to an ANSI cursor-up in the
console output was proposed and declined: real SCF does not rewrite outgoing
characters, and on real OS-9 with a VT100 attached the game misbehaves the
same way. Making one program look right is not worth inventing behaviour OS-9
does not have, in the part of the system whose value is fidelity. (Rebuilding
`robots` from its recovered source fixes it without any emulator change.)

### `tmode baud=` changes PD_BAU -- Kept as is

*Using Professional OS-9* lists `baud` among parameters that "cannot be
changed by `tmode`". Under os9exec it can, per path. This is load-bearing: a
`/tN` bound to a real host serial port runs at the speed in the path's PD_BAU
(`dc74e19`), and `tmode baud=` is how a live port is retuned. The OS9Tests
case "hostterm: tmode baud= retunes a live host port" covers it. The manual
describes a device-descriptor constraint of real hardware that the host
terminal layer does not share.

### Keyboard echo bypasses output pacing and XOFF -- Kept as is

Echo of a typed character goes straight to the screen, skipping the baud
pacing FIFO and any XOFF hold, so it can appear ahead of output still queued
(occasionally inside an escape sequence). Routing echo through the FIFO would
add latency to every keystroke for a cosmetic gain. Echo during a hold is also
faithful: terminals of the era had local echo that could not be turned off.

Do not "fix" this by freezing input during an XOFF hold. That was the old
behaviour and it was wrong: the Technical I/O Manual (PD_XOFF) halts output
only. Corrected in `56f64f7`; OS9Tests "console: XOFF halts output but input
is still taken".

### Flushing paced output on `kill <pid>` -- Declined

Ctrl-C and Ctrl-E flush a killed process's pending paced output, through a
hook scoped to the keypress. Doing the same for an explicit `kill <pid>` was
tried and reverted: `kill_process()` is shared teardown that ordinary `F$Exit`
also uses, so the flush fired on every exit. Doing it properly needs a real
distinction between an abnormal kill and the function's other callers.

### SCF paths do not enforce the read/write mode (E$BMode) -- Kept as is

By default a process's standard input, output and error share one console
path opened for reading, so enforcing the access mode on SCF would refuse
ordinary output. Doing it properly means opening the standard paths in update
mode first, as real OS-9's `/term` is. Not worth the churn while nothing is
known to depend on the refusal. Likewise, I$Attach of a device that does not
exist succeeds; it is a known stub.

### I$Open of `/pipe` in mode 0 answers E$FNA -- Kept as is

The manual says an I$Open of an unnamed pipe opens a new anonymous pipe,
whatever the mode. os9exec refuses mode 0 with `E$FNA`, because that is what
makes `dir /pipe` work: Microware's `dir` opens its argument in mode 0 first
and retries in directory mode on `E$FNA`. Opening a pipe on the first attempt
would hand `dir` an anonymous pipe. A read or write mode opens an anonymous
pipe as documented. Only real hardware can say what OS-9 does with `dir`'s
sequence.

### I$Seek on a pipe succeeds and does nothing -- Kept as is

The Technical I/O Manual's I$Seek entry says managers that do not support
random access "usually do nothing" and return no error; os9exec's pipes do
exactly that. No CONF68K test pins it, because "usually" is not a
pipe-specific claim (SBF, for one, is documented to return `E$UnkSvc`).

### SS_Size on a pipe is refused with E$UnkSvc -- Done

os9exec used to answer the pipe buffer's capacity, and `less` then stopped
reading piped input at 4K. The manual reads both ways: I$GetStt lists SS_Size
for pipes, and pipeman's own list says it returns the buffer size. Any size
cuts piped input short (a real pipe buffer is small), and zero makes `less`
show an empty file, so only an error fits the programs people run
(`2a95c75`). CONF68K t111 asks real hardware, marked as an inference.

### I$Open with access mode 0 -- Kept: the open succeeds, reads fail

The I$Open page (Technical Manual, page 2-16) allows a non-directory file to
be opened with no mode bits so its attributes and size can be examined, and
says such a path permits no I/O. os9exec does both: the open succeeds and a
read answers `E$BMode`. Neither half should be "fixed". CONF68K t04 and t05.
A `curses` module that opens termcap this way (most likely C written with
POSIX `O_RDONLY`, which is 0) is broken on real OS-9 too.

### The `OS9T` wildcard is retired -- Done

`OS9T=pty` once meant "any `/tN` not named individually": the first open of an
unnamed terminal allocated a pty. It is gone. A `/tN` is a device only if
`OS9T<n>` names it or `iterm` declares it (`f107fc1`); anything else is
`E$Unit`. The wildcard created a resource rather than discovering one, so it
could never fail, and a typo such as `>/t5` for `>/t4` silently opened a
terminal nobody was attached to. Real OS-9 has no device without a
descriptor, so the refusal is also the faithful answer.

### "Device not there" keeps two error codes -- Kept as is

An unconfigured `/tN` answers `E$Unit` (there is no descriptor); `/p` with no
printer behind it answers `E$MNF`. Both are defensible, and real hardware has
not said otherwise.

### No device descriptor modules are presented -- Declined

os9exec does not make a descriptor module for `/dd`, `/hN` or `/term`, and
will not fake them. `xmode /term`, `link dd` and similar tools get `E$MNF`
although the devices work. Loading a real descriptor module from disk makes
it resident when a tool needs one.

### A host-bound `/tN` takes its line format from PD_PAR -- Done

A `/tN` bound to a host serial port has always taken its speed from PD_BAU.
It now takes parity, bits per character and stop bits from PD_PAR as the
Technical I/O Manual lays it out (SCF, page 3-9), at open and on every
SS_Opt, so `tmode par= cs= stop=` retunes a live port (`c894dad`). Codes the
manual leaves unassigned (parity 2, stop bits 3) leave that part of the port
alone. termios has no one-and-a-half stop bits; a UART gives 1.5 only at five
bits per character when asked for two, so 1.5 and 2 both set two. Verified
against a pty, which stores the settings; a real serial line is still
unverified, as the speed has been.

### The browser build always signs on; `-l` shortens it to one line -- Decided

A web page is an interactive session whatever program it starts, so the
browser build always shows the sign-on with the original authors' names. A
page that embeds a small terminal (a man page in a catalogue, say) can pass
`-l`, which keeps one line naming os9exec, its version, the authors and the
GPL, and drops the platform and CPU-table lines (`3c16fa5`). At least one line
always remains.

### `printer.c` and the non-console fallbacks -- Not touched

These carry their own hardcoded carriage returns, and `printer.c` adds
line-feeds inside I$Write, which the manual says passes data unmodified.
Neither is compiled by any current build: `printer.c` is inside a CodeWarrior
Windows conditional, and the other fallbacks are behind `#ifndef
TERMINAL_CONSOLE`, which every build defines. Editing code no toolchain can
compile, to fix behaviour no test can reach, was judged the riskier change. If
such a target is ever revived, start here.

### A bound `/tN` with undrained input does not spin -- No change

A review reasoned that a `/tN` holding a byte nobody reads would keep the idle
wait's `select()` ready and burn CPU. Measured on the build in question, it
did not (under 1% CPU with a byte left unread and with the far end closed). A
guarding change was written and reverted as unproven. Revisit only with a
reproduction.

---

## Processes, scheduling and system calls

### os9exec ends when only input waiters remain -- Done

Once the process os9exec was started with has exited, os9exec ends as soon as
every remaining process is only waiting for input (a socket, terminal or pipe
read, or a child). Their connections close with it; refusing to end would be
the worse failure. A process that is running or sleeping keeps os9exec open,
so a background job can finish. `ShutdownDue` in `procstuff.c`; the OS9Tests
"shutdown: ..." cases.

### F$Fork runs only object code; BASIC I-code goes to RunB through the shell -- Done

"To be loaded, the module must be program object code" (F$Fork, Technical
Manual, page 1-30), so F$Fork of anything else is `E$NEMod`. A packed BASIC09
module is `$0202` (Subroutine, I-code); it used to fail as "bad module ID",
because its missing data table was read before its type. Running I-code under
RunB is the shell's job, on the 6809 as on the 68000 ("If the load succeeds
and the module is BASIC I-code, execute Runb command", *Using Professional
OS-9*, the `shell` utility; the 6809 User's Guide, 4.8). Microware's shell
does it under os9exec. os9exec's own launch (`os9exec hog` on the host
command line) stands where a shell would, so it does the same, by name or by
path, handing RunB the module's own name, since RunB takes a name, never a
path. A program that forks a packed module directly gets `E$NEMod`, as on
real OS-9.

### The manual's permission rules are enforced -- Done

An argument audit found six rules the v2.4 Technical Manual states that
os9exec did not enforce, and all six now are: F$Send of S$Kill only within the
sender's group, F$SPrior only on the caller's own user ID, F$SetSys only for a
super user, F$DExit only on an F$DFork child, RBF directory opens and I$ChgDir
needing the permissions the mode asks for, and SS_FDInf in user state only
for the super group. They change nothing for a super user. Where the manual
names no error code the closest documented one is used.

### The system tick: SIGALRM on Unix, the CPU loop elsewhere -- Done

The tick (100 Hz by default, `-q` to switch it off) pre-empts a process
computing without system calls, as OS-9's clock does: "User-state routines
are time-sliced" (Technical Manual, 2-2). On Unix it is a one-shot SIGALRM,
unblocked at start because some launchers start programs with it blocked
(`5b26bd2`). Windows and WebAssembly have no SIGALRM, so the emulation loop
reads the clock itself every 4096 instructions (`b31f713`), a decrement and a
test per instruction there and nothing on Unix. CONF68K t115.

### A sleeper is woken at its deadline, not on a rota -- Done

The scheduler looked at a sleeping process only every 30 arbitration rounds;
beside a process computing without calls a round is a tick, so F$Sleep
overran by up to 30 ticks, and by seconds on a host whose timer is slow. A
sleeper whose deadline has passed is now looked at at once; the rota remains
for the rest (`34d5b25`).

### Signals raised outside a system call are delivered at the tick -- Done

A typed Ctrl-C or Ctrl-E raises its signal outside any system call, so it
waits in the signal queue, which was let out only after the running process
made a call. A program computing without calls never makes one, so the keys
went nowhere. The tick now lets the queue out as a system call does
(`1c95956`). What happens next is the program's or the shell's business: a
program with an intercept routine gets the key; from the shell, Ctrl-E ends a
child without one and Ctrl-C sends one that has written nothing to the
background, as the manual says.

### The idle wait sleeps until something is due; the cap is one tick -- Done differently

The emulator used to poll every millisecond while idle, walking every terminal
slot: about 1.6% of a core doing nothing. An earlier assessment deferred the
fix, believing it needed a `select()` over every terminal's input. It did not:
a terminal's input from another guest process arrives while a guest runs, so
the only host wake sources are standard input, bound `/tN` endpoints and
sockets armed for a signal. `DoWait` now waits on those until the next paced
character, sleeper or alarm.

The cap on a single wait is one system tick (10 ms, `IDLE_CAP_US`). A 50 ms
cap was tried and reverted: it made "console: XOFF halts output but input is
still taken" fail about one run in three, which one green run had hidden. At
one tick the idle cost is a few tenths of a percent of a core. Where nothing
watches input (standard input is a pipe, as under the test harness), the wait
stays at 1 ms, because the nap length then is the input latency. Parked
readers and writers are retried as soon as the host reports input
(`retry_parked_now()`), rather than waiting out a retry rota tuned for the old
1 ms loop.

### Period-accurate CPU speed -- Left open

os9exec paces terminal output to the path's baud rate but runs the 68000 flat
out. Limiting the emulated CPU to a period speed would save host CPU,
especially with several `/tN` terminals live. Not declined on merit, just not
done. Two findings for whoever takes it up: baud pacing is already efficient
(it sleeps exactly until the next character is due), and `-r` currently means
"no output pacing", which the test harness relies on; decide whether CPU
limiting widens `-r` or gets its own flag before writing code.

### `iquit` arms a quit; `stop` exits -- Kept as is

`iquit` sets a flag honoured at the next debugger entry (`F$SysDbg`), and says
so in its help. It does not end a session whose processes are sleeping
indefinitely. `stop` (or `shutdown`) exits immediately and covers the
user-facing need. If `iquit` is ever wanted as an immediate quit, the fix is
to honour the flag in the arbitration loop.

### A module that is not re-entrant is linked by one process at a time -- Reversed later: now enforced

F$Link (Technical Manual, page 1-41): "If the module requested is not
re-entrant, only one process may link to it at a time", with `E$ModBsy`.
This was first declined, on the reasoning that data modules are made to be
shared, that a loaded module already holds a link, and that os9exec has no
memory protection for the rule to protect.

It is now enforced (`8d7d870`). The process whose link found such a module
unlinked becomes its holder; another live process's F$Link, F$Load or F$Fork
of it gets `E$ModBsy`. One process linking twice is not refused, since the
manual names another process. CONF68K t114 uses a data module created without
the re-entrant attribute. None of the freeware disk's modules is affected.

### F$Event: a full table is E$MemFul, and names get little syntax checking -- Kept as is

Ev$Creat lists `E$EvFull`, but the manual's error table never gives it a
number, so it cannot be returned; a full table answers `E$MemFul`, the nearest
documented meaning. The manual defines an event name only as "max 11 chars",
so applying F$PrsNam's character set would be a guess about the kernel. An
empty name, which every OS-9 name rule refuses, is `E$BNam`.

**Cost accepted:** a program testing for `E$EvFull` by number cannot be
satisfied.

### F$SchBit, F$AllBit and F$DelBit: the unspecified cases -- Kept as is

Implemented in v4.1.0 (CONF68K t97, t98). Where the page is silent or
self-contradictory:

- **A failed search.** The FUNCTION text says the carry comes back with the
  largest block found; the ERROR OUTPUT says d1 holds an error code. os9exec's
  dispatcher writes the error into d1 for every failing call, so d0 gets the
  largest run's first bit and d1 gets `E$Full`. Following the FUNCTION text
  would mean teaching the hottest path in the emulator "carry, no code".
- **A successful search** reports the bits asked for, never more.
- **Zero bits** succeeds where it started. Bit 0 is the top bit of byte 0, as
  in RBF's own map.
- **Maps longer than 8192 bytes.** d0.w is sixteen bits, so a bit past 65535
  cannot be named; the scan stops there rather than report a truncated number
  that points at a bit in use. CONF68K t103. Real OS-9 has the same sixteen
  bits.

The map's bounds are checked only against the emulated arena, like F$CRC's.

### The documented call surface -- three F$ calls answer E$UnkSvc, on purpose

Every call in the v2.4 manual's chapter indexes was checked against the
dispatch table. All sixteen I$ calls are implemented. Of the F$ calls, three
are not: `F$SSpd` (its own page says it "is currently not implemented"),
`F$UAcct` (a user-defined call installed by a site's OS9P2 module) and
`F$Trans` (translation to an external bus, which os9exec does not have). Note
for any future sweep: the index spells `F$SigMask`, the table `F$Sigmask`;
fold case before believing a name is missing.

### F$SetSys: hardware-only globals answer 0 -- Kept as is

os9exec answers the system globals it genuinely has (initialisation time, date
parts, process pointers, event IDs, the live process count and so on).
Globals naming hardware or kernel machinery it does not have (interrupt
stacks and polling tables, cache depths, CPU descriptor lists, debugger hooks,
dispatch tables, process queues) stay 0 on purpose: an invented address would
be worse than an honest zero. Offsets come from what a freely distributed
`getsys` utility reads and labels, not from licensed headers.

### F$STrap does not install the FPU exception vectors -- Declined for now

The installable range is vectors 2 to 11, matching the ten slots OS-9 keeps
per process (CONF68K t57, t58). The floating-point set (vectors 48 to 54),
which OS-9 keeps in separate tables, is refused. Nothing in the 68000 core
raises those vectors, so accepting a handler would trade a refusal for
silence. Do it when the FPU emulation can raise them, with its own table
rather than a wider `NUMEXCEPTIONS`. Separately, F$STrap does not validate the
handler stack before building the frame: a bad stack faults and is reported
as a bus error, as on hardware, and the host is not at risk.

### The super user needs module read permission like anyone -- Kept, marked as an inference

os9exec checks the super-user group through the module's owner bits, following
*The OS-9 Guru* (3.2.4): a module with access `$0000` is refused `E$Permit`
even to user 0.0. Some accounts have group 0 bypass the check. No manual to
hand settles it; CONF68K t113 asks real hardware. F$Fork and F$Load apply the
same rule as F$Link (`289d55e`).

### `cp->func` carries three meanings -- Not refactored

One field holds an OS-9 function code, an exception vector offset (used as
`func >> 2`), and 0 meaning "fatal, no handler installed". The last collides
with F$Link, which is function code 0. Only tracing was affected, and it was
fixed without touching dispatch by printing the name captured at entry
(`dbgfunc`). Anyone refactoring: the exception-vector use is the trap, and a
separate field per meaning is the honest shape.

### `-d` tracing goes to the operator; per-process notices go to the process -- Done

Trace output from `-d` used to follow the guest's standard error, so a guest
that redirected its errors got emulator trace inside its own files (or, worse,
inside a pipe it was reading). Trace now goes to the emulator's console
(`dbg_printf`), still honouring the debugger's output redirect. Per-process
notices, such as the unimplemented-call message, deliberately still go to that
process's own standard error: a message about a process belongs to its user,
while a `-d` flag is typed by the operator. OS9Tests "debug: -d tracing goes to
the operator, not into the guest's file".

### Debugger listings go to the operator -- Done

The emulator debugger's P, M, F and V listings go to the operator while its
menu runs, not along `idbg`'s redirected standard output. Internal commands
outside the debugger are unchanged.

---

## Memory and modules

### Colored memory is ignored -- Kept as is

F$Load and F$SRqMem ignore the colored-memory bit. The v2.4 Technical Manual
says colored memory lists "are not essential on systems with RAM consisting
of one homogeneous type", and os9exec has exactly one arena from the host.
Honouring the bit would invent a distinction the machine does not have.
**Cost accepted:** `F$Trans`, which the same passage says needs colored memory
lists, is out of reach; nothing has asked for it.

### F$CpyMem refuses a source outside the arena -- Kept as is

F$CpyMem may read any address inside the emulated memory, whoever owns it,
which is what the manual's "view any memory" allows. A source range outside
the arena is refused with `E$BPAddr`: on real hardware that read would be a
bus error, but here it would fault the host. The destination is checked as
F$ChkMem would check it, as the manual requires.

### Other call buffers are checked against the arena only -- Kept as is

F$GBlkMp, F$AllBit and F$DelBit check their buffers only against the arena,
while F$CpyMem's destination gets the stricter own-memory rule. Real OS-9
without an SSM does not check a system call's buffer against the caller's
memory at all, so the stricter rule is applied only where a manual page asks
for it. The arena check is there for the host's safety.

### Low memory reads normally; the bus error is at the arena's end -- Kept as is

The 68000 core faults only at or past the end of the arena; address 0 and all
low memory read normally (as zeros). Programs that read low memory by mistake
therefore behave differently from real hardware, where the exception vectors
live there, but a bus error on a genuinely bad read is the right answer and
nothing is changed. See "Reported, and not os9exec" below for two programs
this explains.

### No union of the module header structs -- Declined

A union of `mod_exec`, `mod_dev` and `mod_trap` was proposed to make the
casts between them defined. Measured, only two such casts remain, and a union
would not make them defined: the C11 common-initial-sequence rule applies
only when a union object actually contains one of the structs, and a module
here is bytes in the emulated arena. A union would change the spelling and
suggest the question was settled. The real fix is reading guest memory
through byte accessors throughout, which is a project of its own. Both casts
rest on a type check made just before them, and say so at the site.

---

## Networking

### Networking is built from observed behaviour, not from licensed headers -- Decided

os9exec serves the socket interface Microware's networking programs use
(`spfsock.c`). It was built only from what those programs are observed to ask
for when they run under os9exec, never from Microware's licensed headers or
source, and the file says so at its top. The rule is never to be the first to
publish Microware's intellectual property. The programs measured working are
listed in the README's Networking section: `telnet`, `ftp`, `ping`, the
`telnetd` and `ftpd` servers, RPC and others. The older socket library's
programs (`ttcp`, `nslookup`, `nsquery`) are served too; that interface was
already published by the original authors' GPL code.

### The original ISP network stack is retired -- Done

`network.c` and its platform files emulated Microware's Internet Support
Package. They had not been built by any maintained makefile for years and had
rotted to errors. `spfsock.c` does the job against the real networking
software, so the old stack, its `NET_SUPPORT` flag and its path type were
deleted in September 2026. Nothing in the running emulator changed; every
removed line was behind a flag no build defined. Two small remnants of the
original TCP support remain in `funcdispatch.c` and `icalls.c` and are
harmless.

### No ISP-format `inetdb` is provided for `inetd` -- Declined

An `inetd` built for the older ISP stack gets past every socket call but stops
at "tcp protocol unknown": it parses an `inetdb` data module in the ISP
layout, and the only `inetdb` available is SPF's. Building an ISP-layout one
would mean deriving and embedding an unpublished Microware data format, which
the rule above forbids. A user who has a real ISP `inetdb` can load it.

---

## Platforms and builds

### 32-bit Windows -- Reversed later: now executed, not only compiled

For a long time 32-bit Windows was supported on compile evidence alone: the
i686 MinGW leg of `make warnings`, which found a pointer-width bug and a
missing `__stdcall` the day it was added (`d7a4ebd`, `aca7b7f`). For v4.1.0
the conformance suite was run on it too, on Windows 11 through its x86
emulation, and passes. Treat an i686 build failure as a broken platform, not
toolchain noise.

### Native Windows with the MSVC ABI (clang-cl, cl.exe) -- Left open

Windows is supported through MinGW (x86_64, i686 and ARM64). The MSVC ABI is a
platform port, not a recompile: the 68000 core compiles under clang-cl, but
the platform layer needs `dirent` and `termios` console handling
reimplemented for Win32 (an earlier port, `msdir.c`, was deleted in
`992f55e`), and the makefiles would need translating. The Windows build also
lacks networking and host terminals for `/tN`; the README points to Docker or
WSL2 for those.

### MinGW builds get neither PRINTER_SUPPORT nor NATIVE_SUPPORT -- Declined

The CodeWarrior Windows and Linux configurations enable them; the MinGW build
does not. Turning them on is a feature choice, not a hardening fix, and
nothing has asked for them there. Note there are several `target_options.h`
copies in the tree; confirm which one a build uses before editing.

### Undefined behaviour in the UAE 68000 core -- Recorded, not fixed

The rule is "do not touch the UAE core", not "do not look". UBSan finds
eight sites: shifts by a negative exponent (`readcpu.c`, `newcpu.c`, where the
operand is provably 0 in those cases), left shifts into the sign bit
(`maccess.h`, `cpuemu.c`) and a left shift of a negative value. All are
value-benign on two's-complement hardware today; the exposure is an optimiser
entitled to assume the shifts are in range. The upstream-friendly fix, if ever
wanted, is unsigned operands and explicit `>= 0` guards on the two loops.

---

## The 68000 core and FPU

### The CPU is reported as a 68020 -- Done

The core is a 68020 with a 68881, and F$SysID always said 68020, but
`D_MPUTyp` and the built-in `init` module's CPU field said 68040, a value
carried over from the Macintosh build. All three now say 68020 (`c461e5d`),
so a program choosing code by CPU type does not take a 68040 path the core
does not implement.

### FCMP of equal negative values sets N -- Kept, marked as an inference

The M68000 Programmer's Reference Manual's FCMP table leaves N "as
appropriate" for equal negative operands, but sets it for two negative zeros,
where IEEE subtraction gives +0. So os9exec sets N from the operands. CONF68K
t112 asks real hardware.

---

## Testing and verification

### Best guesses get a CONF68K test marked INFERENCE

Where the manuals are silent or disagree and a choice had to be made, the
choice gets a CONF68K test that reports what it observes, and its claim is
marked INFERENCE (t105, t110 to t113 among them). The suite is written to run
on real OS-9/68k hardware; a different answer there tells us which way to go.

### Record-locking tests can report 901 on a heavily loaded host -- Suite left alone

CONF68K's record-locking tests use a readiness handshake that marks a child's
intent to make its contended call, not the moment it blocks; nothing a process
can do observes its own impending sleep. If the child is descheduled in that
gap for the whole release delay, it honestly reports that it got the lock
only after the holder released (obs 901). This was seen on a slow emulated
riscv64 guest, independent of os9exec's version, and then not reproduced in
fifteen runs. The suite was left unchanged, since changing a suite about to
go to real hardware is the bigger risk. A child-side timestamp could tell the
artifact from a real finding if it is ever needed.

### No regression test for paced-output Ctrl-C attribution -- Accepted

The fix that makes paced output carry its writer's process ID was verified by
measurement (a paced writer survived its own Ctrl-E in 3 of 9 runs before, 0
of 9 after). The best trigger found detects a regression only a third of the
time, so a useful test would add about 25 seconds to a one-minute suite. A
single-shot version would pass on a broken build two times in three; do not
add one.

### Console output checks must run without `-r`

`-r` turns output pacing off, which is exactly the condition under which
pacing bugs cannot appear. A pacing bug that cut an internal command's output
short (`def7d6b`) was once misread as a missing `mount` option because of
this.

### Checks must be able to fail

Several checks in this tree were found unable to fail. Two OS9Tests cases
passed by timing out: `sleep 0` sleeps indefinitely (F$Sleep), so a bare
`sleep 0` never returned and "no error appeared" held. They now background the
sleeper and assert something true of one ("sleep: an indefinite sleeper leaves
the shell usable"). A new check should be seen to fail once before it is
believed; `make warnings` prints `NOT BUILT` for a leg that produced no
binary, for the same reason.

### Suite programs keep their data in vsect storage

OS-9 starts a program with a6 pointing $8000 past the start of its data area.
Test programs that used plain `(a6)` as a buffer wrote far past their own
memory, into whatever module was loaded next, which once made `dcheck` appear
to loop. All suite and CONF68K programs now use vsect storage (`115d459`,
`9e64774`). os9exec's a6 was correct.

### The conformance script does not inherit the operator's devices

`tools/conformance.sh` strips every `OS9*` variable the operator has exported
and passes only what it names, except `OS9DISK`, which is the one piece of
environment the suite is meant to take. Otherwise a device mounted in the
operator's shell silently appeared in every run. The Swift suite builds each
emulator's environment explicitly and was already clean.

### One slow test has its own timeout in containers

"dsave: generates script" walks about 2,100 entries of the system disk. When
that disk is bind-mounted into a Docker container from macOS, stat calls are
about five times slower, so the test has a 900-second budget in containers
rather than the suite default. The output is identical; a slow bind mount
looks exactly like an emulator that is slow on Linux, so time the same
command natively before believing the second.

---

## Reported, and not os9exec

These were investigated as possible emulator defects and traced to the
program or its library. They are recorded so they are not reopened.

- **`robots` screen debris.** A bare `^K` used as cursor-up (see "os9exec does
  not translate terminal control codes" above). Rebuilt from source, it is
  clean.
- **`copy`'s "Overwrite (yes/no/all/quit)?" prompt**, which the v2.4 manual
  does not document. The prompt is inside Microware's `copy` module itself; it
  is version drift between the manual and a later edition of the utility.
- **`lpq` reports "No Spooljobs for printer".** Its source passes a single
  character where `strcmp` wants a string, so it reads from an address below
  256. os9exec's low memory is zeros, so the compare matches and every job is
  skipped; on real hardware it would read the exception vectors instead.
- **`snake` bus error without TERM.** A scan for a carriage return runs off
  the top of zero-filled memory. Real RAM would usually hold a CR sooner. Run
  it from a login session with TERM set.
- **`vis` hangs at pipe end of file.** The same small C program, built with
  and without the trap library, reaches end of file only without it; direct
  syscall probes of the same pipe sequence all get `E$EOF` as the manual
  describes. The difference is the trap library build, not the kernel's pipe
  layer. Build such programs without it.
- **Ctrl-E does not stop a packed BASIC09 program with no `ON ERROR`.** The
  signal reaches RunB's intercept routine by either route (directly, or
  forwarded by the shell); RunB turns it into a trappable error only when an
  `ON ERROR` is armed, and otherwise carries on. The manual says RunB lets
  `ON ERROR GOTO` trap Ctrl-C and Ctrl-E. A trapped Ctrl-E reads `ERR` 0;
  what real hardware reads is unverified.
- **A `curses` module that opens termcap in mode 0** (see "I$Open with access
  mode 0" above). Real OS-9 would refuse its reads too.
