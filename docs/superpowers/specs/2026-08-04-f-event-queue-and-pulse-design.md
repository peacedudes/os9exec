# F$Event queue and Ev$Pulse — design (68k)

Written 2026-08-04. Closes the last two open entries in `ROADMAP-68k.md`:
`Ev$Pulse` is the one F$Event subfunction still unimplemented, and it is the
one CONF68K claim the two-process harness could not reach.

## Problem

`Ev$Pulse` "sets the event variable to the value passed in d2, the signal
auto-increment is not applied, then the Ev$Signl search routine is executed
and the original event value is restored" (v2.4 Technical Reference Manual,
page 1-25). The pulsed value therefore exists only for the duration of that
search.

os9exec has no search to run. A waiter parks in `pWaitRead` and re-tests the
range itself each time it is rescheduled — always after the value has been
put back — so every waiter would miss every pulse. Nothing anywhere records
which process is waiting on which event with what range. `Ev$Pulse` returns
`E$UnkSvc` rather than accepting the call and waking nobody, which reports
success for work not done.

Two smaller consequences of the same gap:

- `Ev_AllProcs` (the MS bit of the function-code word, "activate all
  processes in range") is masked off and discarded. It cannot mean anything
  when every parked waiter re-tests for itself, because waking everybody is
  already what happens.
- `Ev$Info`'s offsets 24 and 28 are the queue links, and are written as zero.

## Is it worth building?

Recorded because the first answer given was wrong, and a later reader should
inherit the corrected one rather than rediscover it.

The first assessment was "narrowly worth it": `Ev$Pulse` has no known user,
so a mistake in this code could not hurt anything that works. **The second
half of that is false.** Scanning `h0/CMDS` for the `TRAP #0 / dc.w $0053`
sequence finds F$Event called by 13 of 218 shipped Microware modules —
`cc1plus`, `cc2`, `dhcp`, `events`, `hostname`, `ipstart`, `maui`,
`mbinstall`, `mount`, `ndbmod`, `nfsc`, `spfnppdc`, `telnetdc`. The whole
networking stack, `mount`, and Maui, whose own roadmap entry ("get Maui
working, if possible") is still open. Which subfunctions they call is not
statically knowable — `d1` is computed at runtime — but this is not code
nothing runs, and the change must be verified as if the network stack
depended on it, because it may.

The queue's substantive fix is that `Ev$Signl` currently wakes every in-range
waiter instead of the first. The canonical OS-9 semaphore idiom
(`wInc=-1, sInc=+1`) is immune to that, because the wait auto-increment
carries the "one at a time" meaning in the value itself:

| step | real OS-9 | os9exec before this change |
|---|---|---|
| P1 waits [1,max] | value 1→0, runs | same |
| P2, P3 wait | both queue | both park |
| P1 signals | value→1, wake P2, value→0, stop | value→1; P2 retries, sees 1, value→0; P3 retries, sees 0, stays |

The divergence bites when `wInc` is 0, where "wake exactly one" cannot be
expressed as a value at all — which is the shape a "wake one server" event
takes, and therefore the shape to expect in a windowing system or a network
daemon. That case, `Ev_AllProcs`, and `Ev$Pulse` are what this buys.

rdoggett chose to build it (2026-08-04), having first seen the weaker
assessment above and questioned whether the real motive was only wanting to
avoid shipping with a known gap. It is worth building on the merits: the
software this touches is software the project is still trying to get running.

## Design

### Data model

`event_typ` carries the FIFO head and tail. Its existing `void* next` and
`void* prev` fields are dead — nothing in the tree reads or writes them,
verified by grep — and are replaced by `ushort qHead, qTail` holding process
indices, with `MAXPROCESSES` (129) as the null sentinel. That is the same
"no process" convention `currentpid` already uses, so it needs no new idea.

Each waiter's entry lives in its own `process_typ`, not in a table the event
owns:

| field | meaning |
|---|---|
| `ev_id` | the event this process is queued on; 0 = not queued |
| `ev_next` | FIFO link, `MAXPROCESSES` = end |
| `ev_minV`, `ev_maxV` | the resolved activation range |
| `ev_woken` | the search satisfied this waiter |
| `ev_wakeValue` | the event value observed at the moment it was woken |
| `ev_wakeErr` | non-zero if it was woken by the event going away |

`ev_minV`/`ev_maxV` already exist for `Ev$WaitR`, which has to resolve its
relative range once because entry registers are restored on every retry.
Both wait forms now store the range at enqueue, so the search never reaches
into another process's saved registers, and the two paths stop differing.

Everything is cleared at process creation alongside `pW_age` in
`procstuff.c`.

### The search

One static `evSearch(ev, allProcs)` in `events.c`, the manual's four steps
(page 1-26) in order: walk the FIFO from the head; the first waiter whose
range contains the current value has `ev_wakeValue` set to that value, is
dequeued and flagged `ev_woken`; the event value is then updated with the
wait auto-increment; the search continues with the updated value only if
`allProcs`, otherwise it stops.

Four callers, each doing its own value update first:

```
Ev$Signl  value += sInc;                      evSearch
Ev$Set    value  = d2;                        evSearch
Ev$SetR   value  = evSatAdd(value, d2);       evSearch
Ev$Pulse  save = value; value = d2; evSearch; value = save
```

`Ev$Pulse` is that one line. Everything else here is the queue it needs.

### The wait lifecycle, and why the scheduler does not change

First entry into `Ev$Wait` keeps today's shape: validate the ID, run the
device-syspath poll, test the range. In range → apply `wInc` and return. Out
of range → enqueue and park in `pWaitRead` exactly as now.

On a retry, `evWait` no longer re-tests the range. It asks whether it was
woken, and returns the value the *search* recorded.

That split is the whole trick. The wake **decision** — who, in what order, at
what value — happens at signal or pulse time, which is what makes `Ev$Pulse`
observable at all. Only **delivery** waits for the waiter's next slot, where
the existing park/retry machinery already re-runs the syscall with its entry
registers restored. `os9exec_nt.c`, `set_os9_state`, and the retry loop are
untouched.

The device-syspath poll stays on the retry path. It is how a waiter on a
device-ready event makes progress, and it reaches the queue the same way
everything else does, because it works by calling `evSet`.

### Stale entries

The queue holds process indices, and a process can die, be signalled, or
have its slot reused while queued. Two-sided consistency handles it: an entry
is honoured only when `procs[pid].ev_id == ev->id` *and* that process is
still parked. A reused pid has `ev_id` zero from process creation, so it can
never be mistaken for a waiter.

On top of that:

- `kill_process` dequeues explicitly rather than leaving the walk to find it.
- `evDelet` freeing a slot with waiters still on it wakes each with
  `E$EvNTID` rather than stranding them.
- The intercept path (a signal to a parked waiter saves the syscall's entry
  registers and re-runs it on RTE) leaves the process queued, which stays
  correct — it resumes the same wait on the same event.

### Ev$Info

Offsets 24 and 28 stay zero. The queue now exists, but its links are host
process indices and a guest cannot use one as a pointer into its own address
space. The comment saying "os9exec has no event queue" is corrected to say
why the links are still zero.

## Tests

Three new CONF68K tests: t40 and t41 citing page 1-25, and t42 (added
during implementation, see below) citing page 1-26.

**t40evpuls** — single process. Create an event with value 100, `wInc` 0,
`sInc` 5. Pulse it with d2 = 999. `Ev$Read` → expect 100. One test covering
both halves of the sentence: the original value is restored, and the signal
auto-increment is not applied. `wInc` is 0 so the observation cannot depend
on whether a wait increment was applied during a search that woke nobody,
which the manual does not settle.

**t42evsigw** — added during implementation, not in the original plan. The
queue replaced the wake mechanism for every F$Event call, not only the new
one, and no test anywhere had a process woken by an ordinary `Ev$Signl`: t14
is single-process and measures only the arithmetic. The call most likely to
matter to the shipped network stack and Maui was the one with no coverage.
Same shape as t41 with a signal in place of the pulse, reporting what the
parent reads after the child wakes — a signal leaves its value, a pulse
restores the old one, and the child cannot tell the two apart.

**t41evwake** — two processes on `fork.i`, using `ckrole`, `mkmark`,
`ckmark`, `napticks`, `chdone` and `chfail` directly rather than `contend`,
which is specific to record locking. The parent creates the event, forks; the
child links to it by name, drops the ready marker, and waits on range [7,7].
The parent waits for the marker, sleeps `RELDELAY`, and pulses with d2 = 7.
The child reports the value it woke with; expect 7.

This carries the same residual race `fork.i` already documents and mitigates
the same way: the marker says the child is *about* to call, not that it has
blocked, and nothing a process can do observes another's impending sleep.
`RELDELAY` is far larger than the handful of instructions in that gap.

Both are new claims in `DOCS/claims.md`, and the `Ev$Pulse` omission recorded
there is deleted rather than reworded.

## Files touched

| file | change |
|---|---|
| `Source/OS9exec_core/os9exec_nt.h` | `event_typ` queue head/tail; `process_typ` waiter fields |
| `Source/OS9exec_core/events.h` | `evPulse`, `evDequeue`, revised signatures |
| `Source/OS9exec_core/events.c` | `evSearch`, `evPulse`, enqueue/deliver in `evWait`, `evDelet` wake, `evInfo` comment |
| `Source/OS9exec_core/fcalls.c` | `Ev_Pulse` case; `Ev_AllProcs` passed through instead of discarded |
| `Source/OS9exec_core/procstuff.c` | clear waiter fields at creation; dequeue in `kill_process` |
| `test/68k-conformance/SRC/t40evpuls.a`, `t41evwake.a`, `t42evsigw.a` | new |
| `test/68k-conformance/CMDS/` | rebuilt via `conformance.sh 68k --build` |
| `test/68k-conformance/runall` | run the two new tests |
| `test/68k-conformance/DOCS/claims.md` | two claims added, the omission note deleted |
| `test/68k-conformance/DOCS/expected`, `expected-rbf` | both, or the RBF leg reports EXTRA |
| `test/68k-conformance/readme`, `DOCS/ourruns.md` | counts and the two-process test list |
| `tools/conformance.sh` | `MODULES` |
| `ROADMAP-68k.md` | both entries deleted |

## Out of scope

`evDelet` frees an event slot unconditionally, where the manual says
`E$EvBusy` unless the link count is zero — and `Ev$Creat` sets that count to
one, so a conforming delete needs a matching `Ev$UnLnk` first. Found while
reading for this design. Fixing it changes the tail of all six existing event
tests and their six prebuilt modules, which does not belong in the same
change as a scheduler-adjacent rewrite. It goes into `ROADMAP-68k.md` as its
own entry, with this citation. The queue work must still survive a freed slot
regardless, which it does by waking the waiters.

## Verification

No commit until all of these are green, and each is run rather than assumed:

- `make warnings` — 0/0 on all four toolchains, and not `NOT BUILT`.
- `swift run --package-path test OS9Tests` — currently 192/0.
- `tools/conformance.sh 68k --build` and `tools/conformance.sh 68k --noshell
  --rbf` — both clean, no NEW / CHANGED / MISSING / EXTRA.
- **t40 and t41 made to fail once before they are believed.** Stub `Ev$Pulse`
  back out to `E$UnkSvc` and confirm both report FAIL, then restore. Three
  checks in this repo have been found unable to fail; a new test that has
  never failed is not yet evidence of anything.
