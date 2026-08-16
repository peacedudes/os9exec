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
instruction is interpreted. The suite does not survive that, and it is worth
recording how it fails rather than what it scored.

Two runs completed and each reported two locking failures, but not the same two:
t21 and t23 on one, t19 and t21 on the next, same binary, idle host. The obs=
value was 901 both times, which is the suite's own marker for "the access
succeeded only after the holder let go". So the lock deferred the contender
exactly as claimed; the contender simply lost a race against the holder's rescue
timer. That is a statement about machine speed, not about locking.

Three further runs never got that far: `mount -k` could not produce an 800K
image inside conformance.sh's 60-second budget, so the RBF leg reported "could
not create". Earlier runs had squeaked past the same budget. Anything measured
here is dominated by timeouts.

Read it as: riscv64 BUILDS clean (0 warnings, gcc 15.3), and this environment
cannot run the suite reliably enough to give a verdict either way. A 901 or a
"could not create" from a full-system emulator means rerun on real hardware, not
disagreement with the manual. s390x, which is emulated per instruction rather
than per machine and is far quicker, runs the whole suite cleanly.

An earlier riscv64 run reported 44/44. That was a single sample, and this file
should not have carried it as evidence.
