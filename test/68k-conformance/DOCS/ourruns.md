# What CONF68K does on the systems we can reach

This records our own runs. It is the thing a run on real OS-9/68k hardware
gets compared against — and it is **not** a standard. Every verdict here is
what one emulator did; where it disagrees with `claims.md`, the manual is
right and the system has a defect.

`DOCS/expected` holds the same RESULT lines in machine-readable form, which
is what `tools/conformance.sh` checks a run against.

## os9exec, 2026-07-30

Built from branch `arm64-uae-integration`, macOS arm64 host, 68020/68881 CPU
configuration, system tick on (the default).

Run two ways, because the difference matters:

| Device under test | Result |
|---|---|
| host-native directory (`OS9H8=` a host folder) | 12 PASS, 0 FAIL, 0 SKIP, 0 ERROR |
| real RBF image (`mount -k=500k`, built inside the guest) | 12 PASS, 0 FAIL, 0 SKIP, 0 ERROR |

```
RESULT t01 PASS  obs=000216 exp=000216  I$Open of an absent pathlist reports E$PNNF
RESULT t02 PASS  obs=000211 exp=000211  I$Read of an exhausted file reports E$EOF
RESULT t03 PASS  obs=000203 exp=000203  I$Write on a read-mode path reports E$BMode
RESULT t04 PASS  obs=000000 exp=000000  I$Open with no mode bits set succeeds
RESULT t05 PASS  obs=000203 exp=000203  I$Read on a no-access-mode path reports E$BMode
RESULT t06 PASS  obs=000004 exp=000004  I$Read of more bytes than remain returns a short count
RESULT t07 PASS  obs=070005 exp=070005  write after a seek past EOF extends the file to seek+count
RESULT t08 PASS  obs=000211 exp=000211  a read at a position past EOF reports E$EOF
RESULT t09 PASS  obs=000201 exp=000201  I$Read on a closed path number reports E$BPNum
RESULT t10 PASS  obs=000218 exp=000218  I$Create of an existing name reports E$CEF
RESULT t11 PASS  obs=000065 exp=000065  I$Seek to zero rewinds the path
RESULT t12 PASS  obs=000300 exp=000300  SS_Size reports the number of bytes written
```

## t10 found a real defect on its first run

Worth recording in full, because it is the case the suite was built for and
it turned up immediately.

On its first run t10 reported:

```
RESULT t10 FAIL  obs=000000 exp=000218  I$Create of an existing name reports E$CEF
```

Creating a file whose name already existed returned no error at all. Run
against a real RBF image the same module reported `obs=000218` and passed.
So the defect was in os9exec's **host-native** path only — `pFopen` in
`fileaccess.c` deliberately reopened an existing file on create, with a
comment asserting that I$Create is "open-or-create" and that an existing
file is not an error.

The manual says otherwise: I$Create's own FUNCTION text states that an error
occurs if the pathlist specifies a name that already exists, and os9exec's
own RBF path already returned E$CEF. The host-native path was diverging from
both the specification and the emulator's own other half.

Visible consequence beyond this test: the shell's `>` redirect is documented
as create-only and fails when its target exists. On a host-native device it
was silently overwriting instead.

Fixed the same session; `make test` stayed at 183 passed / 0 failed, and
`make warnings` reported 0/0 on all four toolchains (host cc, linux gcc in
docker, mingw x86_64, mingw i686). Both device types now report 12 PASS.

**The lesson for anyone reading a run of this suite:** a host-native
directory is a shim with no real RBF underneath. A file-system claim can
pass there and fail on a real image, or the reverse. Run both when it
matters — `tools/conformance.sh 68k` and `tools/conformance.sh 68k --rbf`.

## What has NOT been checked

- **No run on real OS-9/68k hardware, or on any OS-9/68k other than
  os9exec.** That is the entire point of shipping the suite, and it is
  exactly the gap it exists to close. Everything above is one
  reimplementation agreeing with the manual, which is weaker evidence than
  it looks.
- **The rebuild path has not been exercised end-to-end by an independent
  assembler.** The modules in `CMDS/` were built by the `r68`/`l68` on our
  own system disk, and that is the only assembler they have met.
- **The record-locking claims are only exercised on an RBF image.** t19–t31
  need real record locking underneath, so they SKIP on a host-native
  directory — which is what CI and `--noshell` run. Their verdicts come only
  from `tools/conformance.sh 68k --rbf`, and that is a local gate.
  The `fork.i` harness itself is exercised everywhere, by t33 (the negative
  control, which deliberately holds no lock) and by t41 (Ev$Pulse, which
  needs no file manager at all). An earlier version of this bullet said the
  RBF leg was "the only thing that exercises fork.i", which was already
  untrue of t33 when it was written.

## 2026-07-31 — t19 to t31, the record-locking tests

Thirteen two-process tests were added, and they found six defects. Five are
recorded in `DOCS/known-divergences` (t19, t23, t25, t27, t29); the sixth was
found on the way and has no test, because it
kills the caller rather than reporting anything: `I$SetStt` with `SS_Ticks` on
a non-RBF path jumps through an uninitialised dispatch slot and bus-errors.
That one shaped the suite — `canlock` probes with `SS_Lock(0)` precisely to
avoid it.

A second batch (t28 to t31) followed, chosen so that the expected outcome
of each involves no blocking — a lock's upper bound, the same-process
exemption, a refused delete, and a zero-length release. Three pass. The
fourth is the sharpest finding of the day: t29 shows that one process
holding two paths to the same file is refused with E$DeadLk where the
manual makes that arrangement the recommended idiom and guarantees the two
paths will not lock each other out. Any program using the documented
multi-record pattern gets spurious deadlock errors.

Current results:

    host-native   18 PASS, 13 SKIP
    RBF image     26 PASS,  5 FAIL   (all five KNOWN)
    --noshell     18 PASS, 13 SKIP   (same as host-native)

The most useful single thing in that table is the PAIR t19/t21. Both make the
same demand of the same lock and differ only in whether the second process
reads or writes. t21 passes, reporting E$Lock exactly as documented; t19
reports 902 — its contender never came back at all. That is not a vague
"locking is broken": it says readers are resumed and writers are not, which is
a one-line answer to where to look. Neither test could have said it alone.

The suite's own numbers 901 and 902 appear in this run's obs= column. They are
not OS-9 error codes: 901 means the access succeeded only after the holder
released, and 902 means it never returned and was killed. See the readme.

**These are still os9exec results.** A FAIL here is os9exec disagreeing with
the manual, and nothing in this file is evidence about real OS-9/68k.

## 2026-08-04 — t40 and t41, Ev$Pulse

Ev$Pulse was the last F$Event subfunction os9exec did not implement, and the
one claim in `claims.md` recorded as out of reach rather than merely untested.
Both facts had the same cause: os9exec kept no event queue. A waiter parked
and re-tested the range whenever it was next scheduled, which is always after
a pulse has restored the value, so no waiter could ever see one. The call
answered E$UnkSvc rather than pretending.

The queue was built (`Source/OS9exec_core/events.c`), and with it the search
the manual describes — FIFO order, the first process in range, or every one of
them when the MS bit of d1 asks. Three tests came with it:

    host-native   29 PASS, 13 SKIP
    RBF image     42 PASS,  0 SKIP

```
RESULT t40 PASS  obs=000100 exp=000100  Ev$Pulse restores the original event value
RESULT t41 PASS  obs=000007 exp=000007  Ev$Pulse wakes a process already waiting on the event
RESULT t42 PASS  obs=000001 exp=000001  Ev$Signl wakes a waiting process and leaves its value behind
```

t42 was not in the original plan, and it is there because the change created
the gap it fills. The queue replaced the wake mechanism for EVERY event call,
not just the new one — and until t42 no test anywhere had a process actually
woken by an ordinary Ev$Signl. t14 is single-process and measures only the
arithmetic. The one call the shipped network stack and Maui are most likely to
depend on was the one with no coverage.

All three were made to fail before any was believed, and each fails for its
own reason, which is what makes them three tests rather than one:

| sabotage | t40 | t41 | t42 |
|---|---|---|---|
| Ev$Pulse returned to E$UnkSvc | FAIL obs=000208 | FAIL obs=000208 | PASS |
| pulse keeps set/restore, runs no search | PASS | FAIL obs=000902 | PASS |
| Ev$Signl runs no search | PASS | PASS | FAIL obs=000902 |

The middle row is the one worth reading. A pulse that sets the value and puts
it back, waking nobody, is indistinguishable from a correct one to any single
process — t40 cannot tell them apart and should not be able to. t41 reports
902: its child was never woken, sat in Ev$Wait until the parent's timeout
expired, and was killed. That is the whole reason the test needs two
processes.

t41 and t42 are the first two-process tests here that are not about record
locking, and the first where the CHILD must be blocked before the PARENT acts.
Both report the same verdict on a host directory and on an image, because
F$Event is a kernel call with no file manager under it.

**These are still os9exec results.** A PASS here says os9exec agrees with the
manual; it is not evidence about real OS-9/68k.

## 2026-08-04 (later) — t43 and t44, the Ev$Delet use count

Found while writing the queue above, and fixed rather than filed: `Ev$Delet`
freed an event whatever its use count, where page 1-21 says an event "may not
be deleted unless its use count is zero" and answers E$EvBusy when it is not.
Since `Ev$Creat` sets the count to one, os9exec was allowing a delete that
real OS-9 refuses in the most ordinary case there is — the creator deleting
its own event.

All six earlier F$Event tests ended with exactly that unchecked delete, and
so did the three new ones. They now unlink first, which is what a program has
to do on real OS-9 anyway; the forked children in t41 and t42 unlink their own
link before exiting, since "OS-9 does not automatically unlink events when a
F$Exit occurs".

    host-native   31 PASS, 13 SKIP
    RBF image     44 PASS,  0 SKIP

```
RESULT t43 PASS  obs=000169 exp=000169  Ev$Delet refuses an event whose use count is not zero
RESULT t44 PASS  obs=000000 exp=000000  Ev$Delet succeeds once the use count has reached zero
```

**t44 is the interesting one, and it is not a test we can be sure of.** The
manual contradicts itself: Ev$UnLnk (page 1-20) says the count is decremented
"and the event is deleted when the count reaches zero", which would make
Ev$Delet unreachable by any program. We went with the Ev$Delet page, backed by
the OS-9 Guru ("the event can be deleted by a delete event call") and by OS-9
Insights, whose evdel utility unlinks in a loop until the delete stops
failing — a loop nobody would write if unlinking already deleted. Two of those
three are not Microware.

Both were made to fail before either was believed, and the second sabotage is
the one that matters, because it is the rival reading implemented on purpose:

| sabotage | t43 | t44 |
|---|---|---|
| link-count check removed from Ev$Delet | FAIL obs=000000 | PASS |
| Ev$UnLnk deletes when the count hits zero | PASS | FAIL obs=000168 |

So t44 does discriminate between the two readings of the manual rather than
merely agreeing with the one we implemented. If real OS-9/68k hardware returns
168 here, this suite is wrong and os9exec should follow the hardware — that is
the single most valuable line a run elsewhere could send back.

**These are still os9exec results.** A PASS here says os9exec agrees with our
reading of the manual, which is weaker than agreeing with OS-9.

## 2026-08-15 - full-system emulation cannot give this suite a verdict

riscv64 here runs under QEMU's full-system TCG emulation, where every guest
instruction is interpreted. The suite is not reproducible there, and the useful
record is what the cause is NOT.

Five runs of the RBF leg scored 44, 42, 42, 43 and 41. Every failure was in the
two-process locking family, never the same set twice, on an idle host with one
binary. The obs= value was 901, the suite's own marker for "the access succeeded
only after the holder let go".

That marker suggests an obvious explanation: fork.i's holder releases after
RELDELAY, which is 60 ticks, and os9exec's tick is 100Hz of real time, so the
contended child has 0.6 real seconds to fork, open and block. On a machine this
slow it might simply not get there. **That was tested and is wrong.** Running the
same binary with the tick stretched tenfold (`-q100`, making RELDELAY 6 seconds)
produced 44, 42 and 42 against 43 and 41 at the default tick, with the harness
timeouts held at 600s in both arms. The distributions overlap; the tick is not
the variable. A single 44/44 in the slow arm looked like proof and was not.

Raising conformance.sh's per-invocation timeout from 60s to 600s IS necessary
here - without it `mount -k` cannot produce an 800K image in time and the leg
reports "could not create" before any test runs - but it does not make the
locking results reproducible.

So the remaining suspect is scheduling jitter between the two emulated
processes, where a 100x interpretation penalty lands unevenly on whichever one
holds the CPU. Untested.

Read a 901 from a full-system emulator as "inconclusive, rerun on real
hardware". riscv64 BUILDS clean here (0 warnings, gcc 15.3). s390x, emulated per
instruction rather than per machine, runs the whole suite cleanly and repeatedly.

An earlier riscv64 run reported 44/44 and this file should not have carried that
single sample as evidence.

## os9exec, 2026-09-04: t50, MOVE from SR in user state

t50 was added with the fix it measures, and it failed once first. On the
os9exec of the day before, running it from a shell ended the process at its
first instruction:

```
 Executing: -->00062014: 40c6 ccbc 0000 2000 0c86 MVSR2.W D6
# Exception: pid=3 vector=$08 err=#000:108 -- process will be killed
```

so the suite would have reported t50 as MISSING, which is the shape of this
divergence everywhere: a system that traps the instruction never gets to print.
With the nine MOVE-from-SR handlers in the 68020 core no longer raising the
privilege violation in user state, both legs report

```
RESULT t50 PASS  obs=000000 exp=000000  MOVE from SR completes in user state with S clear
```

and the two RTF Fortran programs that had been dying on this instruction
inside their run-time (`creadoc`, `biory`) run to completion; `biory` writes
its chart. The open question t50 carries -- whether Microware's own 68010-and-
later kernels emulate the instruction the way Motorola advised -- is still
open: nothing we can reach is a Microware kernel on a 68010 or later.

## os9exec, 2026-09-04: t51 to t53, F$Mem

F$Mem was not implemented -- the call came back E$UnkSvc, and Carl Kreider's
`subber` on the freeware disk stopped on it -- so the three tests were written
with the implementation and run against the old dispatch first: all three
reported FAIL with obs=000208.

With F$Mem in place, both legs report

```
RESULT t51 PASS  obs=000000 exp=000000  F$Mem with d0=0 reports the size and top the fork handed over
RESULT t52 PASS  obs=000223 exp=000223  F$Mem contracting under the stack pointer reports E$DelSP
RESULT t53 PASS  obs=004096 exp=004096  F$Mem gives back the top of the data area and regains it
```

t53 did not start out that way. Its first version only expanded, by 4096
bytes, and measured the manual's caveat rather than the kernel: PASS when run
by hand from a shell, SKIP with 207 (E$MemFul) every time from runall on a
host-native directory, PASS from an RBF image -- whether the block above a
process's data area is free depends on what the system allocated before it.
The test now gives 4096 bytes back first and asks for them again, which makes
the space above the area free by construction and the verdict deterministic.

The same accident is real for programs. A C program built with `cc -I` links
`cio` right after entry, and that module then sits directly above its data
area; its first F$Mem expansion is refused with E$MemFul, exactly as the manual
warns. `subber` is one: it stops with 207 on os9exec unless `cio` was loaded
before it ran, and substitutes correctly when it was. On a real system the
trap handler is normally resident already, which is the same condition.

## os9exec, 2026-09-04: t54, F$SysID in its pre-3.0 form

Written with the implementation, and run against the old dispatch first:
FAIL with obs=000208. With the call in place, both legs report

```
RESULT t54 PASS  obs=000000 exp=000000  F$SysID fills three terminated strings and names a processor
```

os9exec answers 68020 for both processor numbers, 1 and 1 for licensee and
serial, and its own version and copyright strings; the freeware disk's `sysid`
utility (hc_utils) prints them. This is the only test in the suite whose claim
comes from neither Microware nor Motorola -- the v2.4 manual does not have the
call -- and claims.md says so. On a v3.0-or-later kernel it is not a verdict.

## os9exec, 2026-09-11: t55, what an F$STrap handler is given

Found by tracing GNU Oleo on the freeware disk, which died on an illegal
instruction after its TRAPV handler ran. The handler resumes by returning
through the program counter the manual says arrives in a0, and os9exec had
never set a0. Run against the unfixed emulator first: FAIL with obs=000001,
a0 wrong while a1 and d7 were already right. With a0 set, both legs report

```
RESULT t55 PASS  obs=000000 exp=000000  an F$STrap handler gets the PC in a0, SP in a1, vector in d7
```

The exception table sits before the handler on purpose. os9exec reads a
table entry's handler offset as unsigned, so a table after its routine
installs the handler 64K too high; whether that is right is a separate open
question, and this test is about the registers.

## os9exec, 2026-09-11: t56, the stack an F$STrap handler runs on

The same trace, one step further. With a0 fixed, Oleo resumed at the right
place and then took a bus error: its handler switches back to the program's
stack and calls ordinary routines there before restoring the register image,
and os9exec had built that image just below the program's stack pointer,
where those calls land. F$STrap's own a0, the stack to use when an exception
occurs, was recorded and ignored. Run against the a0-only emulator first:
FAIL with obs=000001. With the image built on the given stack, both legs
report

```
RESULT t56 PASS  obs=000000 exp=000000  an F$STrap handler runs on the stack F$STrap was given
```

and Oleo draws its title screen. The other program seen dying at the same
address, GSHELL's `editor`, is not this: it fills a 100-entry array from a
directory of any size and its own qsort then overwrites a function pointer.
The shared address was only where each wild jump came to rest in memory the
emulator keeps zeroed.

## os9exec, 2026-09-12: t57, the vectors F$STrap would not accept

os9exec accepted handlers for vectors 2-8 only. The v2.4 manual's F$STrap page
also lists line-1010 and line-1111, and OS-9's own process descriptor has ten
slots for them -- P$Except at $03C and P$ExStk at $064, ten longs each, which
the offsets themselves prove ($064-$03C = $28). So the table was three entries
short rather than deliberately narrow. Run against a binary built from the
previous commit: `Cannot install handler for vector number $0A`, then the
process killed with vector $0A, error 110 (E$1010), and no result line at all.

```
RESULT t57 PASS  obs=000000 exp=000000  an F$STrap handler catches a line-A exception
```

The test asks for line-1010 rather than line-1111 on purpose. os9exec emulates
a 68020 with a 68881, so `$F2xx` is a legal coprocessor-1 instruction: the CPU
executed it, took the following word as its extension, and ran the PC into the
test's own table -- which is what the first version of this test actually
measured. A line-F test on this target would assert something about the FPU
model, not about F$STrap.

Still not implemented, and honestly so: the FPU exceptions (vectors 48-54)
live in their own descriptor tables, P$FPExcpt and P$FPExStk at $338/$354,
seven longs each. F$STrap still refuses them, and nothing in the CPU core
raises them, so accepting the install would buy a program silence rather than
a handler. GNU Oleo asks for all seven, is refused, and runs anyway.

## os9exec, 2026-09-12: t58, the signed table offset

A table entry's routine offset is a word relative to the entry, reaching plus
or minus 32K (The OS-9 Guru, 11.2), so a table placed after the routines it
names carries a negative one. os9exec read it as an unsigned word and installed
the handler 64K above the routine; the process then died at the very exception
it had asked to catch. Before the fix the test dies with a bus error and no
result line; every other test in this suite puts its table first, which is why
nothing had caught it.

```
RESULT t58 PASS  obs=000000 exp=000000  an F$STrap table may sit after the handler it names
```

With t55 and t56 from the day before, the four F$STrap tests now cover what the
manual states: the registers a handler is entered with, the stack it runs on,
the vectors that may be caught, and the sign of the offset that finds it.

## os9exec, 2026-09-14: t59, renaming by rewriting a directory entry

A directory is a file of 32-byte entries, and a program renames a file by
reading to its entry and writing it back spelt differently. Microware's C
library rename() does that, and so do move, wndex and upperdir. On an RBF
image os9exec always allowed it. On a host-native directory every directory
write was E$BMode, so all four failed on a host `/dd` while working on an
image; osk-freeware found them by running the same programs both ways.

Before the fix, on a host-native directory:

```
RESULT t59 FAIL  obs=000203 exp=000000  a directory entry rewritten with a new name renames the file
```

After it, on both a host-native directory and an RBF image:

```
RESULT t59 PASS  obs=000000 exp=000000  a directory entry rewritten with a new name renames the file
```

A host directory has no entries to rewrite, so os9exec turns a write that is
purely a rename into a host rename and refuses the rest: a new or zeroed
entry, another file's FD sector, a name another entry already answers to
(ignoring case, as OS-9 does), a host name the entry does not spell exactly,
and anything open or in use as a current directory. Those refusals are
host-directory policy rather than RBF behaviour, so they are checked by the
integration suite, not here.

## os9exec, 2026-09-14: t60, a process ID past the table

F$GPrDsc indexed os9exec's process table with the caller's d0.w and checked
nothing but whether the slot it landed on looked unused. An ID past the table
read host memory beyond it, so some IDs came back as live processes with a
descriptor made of whatever was there. osk-freeware's sysmon walks IDs until
the errors come back: it was told of processes that did not exist, and at
ID $1A8 it took a bus error. F$DExec and F$DExit had the same unchecked index,
and F$DExit wrote through it; those are debugger calls, so they are checked by
the integration suite rather than here.

Before the fix, one of the four IDs was answered as a process:

```
RESULT t60 FAIL  obs=000000 exp=000224  F$GPrDsc of a process ID past any table reports E$IPrcID
```

After it:

```
RESULT t60 PASS  obs=000224 exp=000224  F$GPrDsc of a process ID past any table reports E$IPrcID
```

What an out-of-range read returns depends on the host's memory layout, so the
"before" answer can differ from machine to machine; E$IPrcID is the only answer
that holds on all of them, which is why this is a conformance test and not only
a local one.

## os9exec, 2026-09-14: t61, two update paths on one sector

Every RBF path keeps its own copy of the sector it is working in, and paths
open on the same file share them through a ring. When one path dirtied a
sector, the ring dropped the other paths' clean copies but kept any dirty
one, "its own unflushed work". Both copies were then written back at close,
and the one written last won. So when path A wrote a byte, path B picked up
A's copy, wrote its own byte elsewhere in the same sector, and closed first,
A's older copy landed on top and B's byte was gone.

osk-freeware found it through move, which gives a file its new name through
one update path on the directory and clears the old name through a second.
On every image the old name survived beside the new one, on the same FD
sector, so deleting either would have freed sectors the other still used.

Before the fix, on an RBF image (the host-native directory has no sector
copies and always passed):

```
RESULT t61 FAIL  obs=000008 exp=000000  two update paths keep both writes, whichever closes first
```

After it, on both:

```
RESULT t61 PASS  obs=000000 exp=000000  two update paths keep both writes, whichever closes first
```

A path dirties a sector only after loading the newest copy, from the device
or from the one path holding it dirty. Dropping every other copy, dirty ones
included, at that moment leaves exactly one unflushed copy, and it already
carries everyone's changes.

## os9exec, 2026-09-14: t62, moving a file by link-then-unlink

move does not copy a file or rewrite its entry. It gives the file a second
name, then removes the first. Through one update path on the directory it finds
the old entry. Through a second it appends an entry carrying the same FD
sector. Back through the first it clears the old entry's first byte.

That failed on os9exec both ways, for different reasons. On an RBF image the
clear was lost: the second path still held its copy of the sector, and wrote
it back over the cleared byte at close. That was fixed by t61's change earlier
the same day, so t62 passes on an RBF image from there on. On a host-native
directory there are no entries, so the append was refused with E$BMode:

```
RESULT t62 FAIL  obs=000203 exp=000000  a file moved by link-then-unlink opens under the new name only
```

A host directory now takes the append as a host rename of the file its FD
sector names. It then treats the clear that follows as already done, but only
as the next directory write by the same process in the directory the file
left; anywhere else a cleared entry is a delete, still refused. On both kinds
of device:

```
RESULT t62 PASS  obs=000000 exp=000000  a file moved by link-then-unlink opens under the new name only
```

## os9exec, 2026-09-14: t63, makdir onto a directory that exists

osk-freeware found it with Microware's makdir on an RBF image: the second
`makdir` of a directory answered E$FNA, where onto an existing file, or onto
a directory on a host-native device, it answered E$CEF. perl 4's mkdir then
reported E$FNA, and scripts testing for E$CEF misread it.

makdir creates through an open that asks for a file. When the name was
already a directory, that open refused it as "not a file" before asking
whether it was creating. Before the fix, on an RBF image (the host-native
directory already passed):

```
RESULT t63 FAIL  obs=000214 exp=000218  I$MakDir of an existing directory reports E$CEF
```

After it, on both:

```
RESULT t63 PASS  obs=000218 exp=000218  I$MakDir of an existing directory reports E$CEF
```

## os9exec, 2026-09-19: t65, braces in a name

F$PrsNam accepted "{" and "}" as name characters, an extension written for
the classic Macintosh MPW build, where the shell substitutes {variables}.
The manual lists the name characters and says anything else ends the name.
Before the fix:

```
RESULT t65 FAIL  obs=000007 exp=000000  F$PrsNam ends a name at any character the manual does not list
```

After it (braces are still accepted in the MPW build only):

```
RESULT t65 PASS  obs=000000 exp=000000  F$PrsNam ends a name at any character the manual does not list
```

## os9exec, 2026-09-19: t69, signals queued while masked came out of order

Three signals sent to a process with its signals masked reached its
intercept routine as 301, 300, 302. The queue itself was first-in
first-out; the fault was in handing one back. When the mask was cleared the
oldest signal was taken off the queue and delivered -- and, where delivery
had to wait a moment longer, put back on the END of the queue, behind every
signal sent after it. It now goes back at the head. Before:

```
RESULT t69 FAIL  obs=003102 exp=003012  signals sent while masked are queued, delivered first-in first-out
```

After:

```
RESULT t69 PASS  obs=003012 exp=003012  signals sent while masked are queued, delivered first-in first-out
```

## os9exec, 2026-09-19: t70, the queued-signal count in d0

OS-9 Insights documents what the Technical Manual does not: an intercept
routine is entered with d0 holding the number of signals queued for the
process, the one being delivered included. os9exec set only d1 and a6, so
the routine saw whatever the interrupted code had in d0. Before:

```
RESULT t70 FAIL  obs=003000 exp=003321  an intercept routine finds the queued-signal count in d0
```

After:

```
RESULT t70 PASS  obs=003321 exp=003321  an intercept routine finds the queued-signal count in d0
```

## os9exec, 2026-09-19: t79, SS_EOF at the exact end of a host file

On a host-native directory SS_EOF asked the host C library whether end of
file had been hit, which it only reports after a read has tried to go past
the end. A path that had read a file exactly to its end -- the ordinary case
-- was told it was not at end of file. RBF images compared the position with
the size and were right. Before, on the host directory:

```
RESULT t79 FAIL  obs=000010 exp=000011  SS_EOF answers 0 before the end of a file and E$EOF at it
```

After, on both:

```
RESULT t79 PASS  obs=000011 exp=000011  SS_EOF answers 0 before the end of a file and E$EOF at it
```

## os9exec, 2026-09-19: t85, a signal did not end a blocked pipe read

A process waiting in a pipe read, interrupted by a signal it intercepts, came
out of F$RTE with the read reported as done: carry clear and a count left
over from the registers. The pipe also went on counting it as a waiting
reader. The signal's own error never appeared, so an alarm used as a read
timeout -- the manual's own example -- could not work on a pipe. Before:

```
RESULT t85 FAIL  obs=000000 exp=000002  an alarm's keyboard abort ends a blocked read with error 2
```

After:

```
RESULT t85 PASS  obs=000002 exp=000002  an alarm's keyboard abort ends a blocked read with error 2
```

## os9exec, 2026-09-20: t91, an absolute alarm set for a time already past

A$AtDate and A$AtJul refused any time earlier than now with E$Param ("alarms
in the past are not allowed", alarms.c). Neither manual page says that, and
the Guru describes the system process firing whatever has been "reached (or
exceeded)". Such an alarm is now due at once. Before:

```
RESULT t91 FAIL  obs=000225 exp=000011  an absolute alarm set for a time already past is still sent
```

After:

```
RESULT t91 PASS  obs=000011 exp=000011  an absolute alarm set for a time already past is still sent
```

## os9exec, 2026-09-20: t92, a sleep a signal should have cut short

A process that arms `A$Set` with S$Wake and then sleeps is its own sender: an
alarm is delivered in the context of the process that armed it. `send_signal`
discarded a self-directed S$Wake outright -- true enough for a process that is
running, wrong for one lying in `pSleeping` -- so the sleep ran its full course,
and an indefinite one would have run forever. That is the pattern F$Sleep's own
page recommends for waiting on a signal. A process trace showed the wake
arriving and being dropped:

```
# send signal=1 to pid=3 (pSleeping) from currentpid=3
```

The test took 5.09 s before the fix and 0.24 s after it. Before:

```
RESULT t92 FAIL  obs=000101 exp=001111  a sleep cut short reports the ticks left
```

After:

```
RESULT t92 PASS  obs=001111 exp=001111  a sleep cut short reports the ticks left
```

t93 and t94, added in the same batch, passed on their first run on both a host
directory and an RBF image: I$ReadLn stops at the end-of-line character and
counts it, and I$WritLn stops at the carriage return.

## t91 was wrong about its own arithmetic, 2026-09-20 (a test defect, not a divergence)

t91 asked for "ten seconds ago" by subtracting 10 from what F$Time returned. That is right for the
Julian form, where the time is a plain count of seconds since midnight, and wrong for the Gregorian
one: `00hhmmss` is three binary fields (F$Time, page 1 - 64), so when the seconds field is under 10
the subtraction borrows. `00:12:04` less 10 became `00:11:250`, which converts to 186 seconds in the
FUTURE -- and the test, which waits two seconds, reported the Gregorian half missing:

```
RESULT t91 FAIL  obs=000010 exp=000011  an absolute alarm set for a time already past is still sent
```

About one run in twenty-five, and it went unnoticed for a day because the suite's own runs happened
to land on seconds above 10. Two instrumented traces pinned it without ambiguity -- the alarm that
never fired had been armed 18,600 ticks ahead:

```
# OS9_F_Alarm: id=00000001 aFunc=3 sig=00000137 tim=00030BFA dat=07EA0914 err=0
# AtDate: iDate=2461303 iTime=11524 aDate=2461303 aTime=11710 -> ticks=18600 err=0
```

The test now asks for midnight today in both forms: a time of zero needs no subtraction, no guard
against a field underflowing, and no reading of the clock to decide whether the guard holds. Midnight
is in the past for all but the first instant of a day, where it equals the current time -- which the
manual's "greater than or equal" makes due as well. 100 consecutive runs pass at load 15 to 24, where
the old test failed about four times in a hundred.

The emulator was never at fault here, and the claim is unchanged. What is worth keeping is the shape
of the mistake: arithmetic on a packed OS-9 date or time is field arithmetic, and a borrow crosses
into a field that means something else.
