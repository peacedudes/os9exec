# Baud-rate output pacing v2: FIFO + host-timed consumer

## Problem

The current baud pacing (`consio.c`'s `ConsoleOut`, shipped in `025a763`/`a4a75ba`)
writes a whole buffer to the terminal instantly via `ConsPutc`, then parks the
writing process afterward for however long that transmission "should" have
taken. The visual result is burst-then-pause — very noticeable at low baud
rates (the whole use case: watching Star Wars or a game scroll by at a
plausible vintage-modem speed). The fractional-tick "owed" accounting is also
an approximation forced by `GetSystemTick()`'s 10ms resolution.

An earlier attempt at true char-level pacing (see project memory
`feature-baud-rate-pacing`) was reverted: it tried to reuse the existing
`pWaitRead` process state for write-blocking, which pulled in read-specific
signal/interrupt side effects (`pwr_brk`, signal-delay logic) that don't apply
to a blocked writer, causing a `$$$$` flood / shell hang. It also had a
pid==129 out-of-bounds landmine (`procs[MAXPROCESSES]`, the sentinel pid used
for system/banner output) and used OS-9-tick-granularity timing, too coarse
for per-char pacing at high baud.

## Goals

- Smooth, character-at-a-time visible output pacing at any baud rate, not
  burst-then-pause.
- The writing OS-9 process genuinely blocks once a small, hardware-realistic
  buffer fills (not an unbounded internal staging buffer that fakes instant
  completion).
- No host-side busy-blocking: other OS-9 processes keep running and Ctrl-C
  stays live while output is paced (matches the existing pacing commit's own
  stated goal).
- One simulated device per physical console (`/term`, future `/t1`, etc.),
  independently paced — not global, not per-open-path.
- No polling overhead when nothing is queued anywhere; no OS-level async
  signal handler (this codebase deliberately avoids mutating scheduler state
  from signal handlers — see the Ctrl-C fix's `HandleEvent()` poll-based
  design).

## Non-goals

- Multi-terminal (`/t1`, `/t2` as independent windows/ptys) is a separate,
  already-deferred feature (see project memory `feature-multi-terminal`).
  This design only needs to make per-device scoping *possible* for that
  future work by keying state off the existing console/device id, not
  implement multi-terminal itself.
- Real serial hardware (`-t1 /dev/ttyUSB0`, project memory
  `feature-serial-t1`) is out of scope; this only concerns the simulated/
  emulated console path.

## Architecture overview

Two independent pieces, connected only through the ring buffer:

- **Producer**: `ConsoleOut` (`consio.c`), running inside whatever OS-9
  process issued the write. Pushes bytes into the target device's ring
  buffer as fast as it wants until the ring is full, then blocks.
- **Consumer**: a host-time-driven drain step, invoked from the scheduler's
  existing cooperative-yield points (`DoWait()`, `do_arbitrate()`) — no new
  thread, no signal handler. Pops one byte at a time, no faster than the
  device's baud rate allows, and calls `ConsPutc` to actually display it.

## Data structures

One `baud_device_t` per physical console, keyed by the existing
`term_id`/`gConsoleID` (the same id `pCopen` already assigns per `/term`,
`/t1`, `/tty00`, etc.):

```c
#define BAUD_FIFO_SIZE 256

typedef struct {
    byte  buf[BAUD_FIFO_SIZE];
    ushort head, tail, count;   /* circular buffer bookkeeping */
    ulong  us_per_char;         /* 10,000,000 / bps; 0 = no pacing (bps unknown or -r) */
    ulong  next_due_us;         /* host time (raw, not GetSystemTick) next pop may happen */
} baud_device_t;
```

A small fixed-size table of these (indexed/looked-up by console id, same
cardinality as the existing console/tty id space) replaces the current
single `static ulong owed` local in `ConsoleOut`.

A single process-wide value tracks the earliest pending deadline across all
devices with queued output (or a sentinel "none pending"):

```c
ulong g_next_wake_us;   /* earliest next_due_us across all non-empty devices, or 0 = none pending */
```

This is what makes the consumer side "one-shot" rather than polled on a fixed
cadence — see Consumer section.

## Producer: `ConsoleOut`

Same per-char transform loop that exists today (case-fold via `_sgs_case`,
CR/LF via `_sgs_eorch`/`_sgs_alf`, page-pause via `_sgs_pause`/`_sgs_page`) —
only the *destination* of each finished byte changes, from an immediate
`ConsPutc(c)` call to a ring-buffer push. Mirrors `pConsRead`'s existing
`pWaitRead` resume pattern exactly (`consio.c:377-397`), reusing the same
per-process fields (`cp->saved_cnt`, `cp->saved_state`) rather than adding
new ones, since a process can only be blocked in one syscall at a time:

```c
if (cp->state==pWaitWrite) {
    set_os9_state(pid, cp->saved_state, "ConsoleOut");
    cnt= cp->saved_cnt;
}

while (cnt<*maxlenP) {
    c= buffer[cnt];
    /* existing case-fold / CR-LF / page-pause logic, unchanged */

    if (!fifo_push(dev, c)) {
        cp->saved_cnt  = cnt;
        cp->saved_state= cp->state;
        set_os9_state(pid, pWaitWrite, "ConsoleOut");
        arbitrate= true;
        break;
    }
    cnt++;
}
```

`fifo_push` also updates `next_due_us`/`g_next_wake_us` bookkeeping for a
previously-empty device (see Consumer section) so the very first queued byte
gets a deadline.

**`-r` (full speed / `baud_throttle==false`) and unknown baud (`bps==0`)**:
same push path (`us_per_char=0`), but the producer calls the drain routine
inline, synchronously, immediately after finishing its push loop, before
`ConsoleOut` returns — so `-r` stays byte-for-byte synchronous with zero
added scheduler-pass latency (matters for the test harness, which already
relies on `-r` for deterministic fast runs), while sharing all the ring
buffer/plumbing code with the paced path.

**Safety**: every new/modified code path guards `pid>0 && pid<MAXPROCESSES`
before touching any `cp->` field, closing the `pid==129` (`MAXPROCESSES`
banner/system-output sentinel) out-of-bounds landmine identified in the
prior attempt.

## Consumer: draining a device

```c
void baud_drain_due(void) {
    ulong now;
    if (g_next_wake_us==0) return;         /* nothing queued anywhere */
    now= host_micros();
    if (now < g_next_wake_us) return;      /* not due yet */

    for each device d with d->count>0 and d->next_due_us<=now:
        c= fifo_pop(d);
        ConsPutc(c);
        d->next_due_us += d->us_per_char;

    recompute g_next_wake_us as min(next_due_us) over all non-empty devices,
    or 0 if all empty
}
```

Hooked into the two existing cooperative-yield points, no new thread:

- **`DoWait()`** (`procstuff.c:958`): the idle-loop `nanosleep` duration
  changes from an unconditional 1ms to `min(1ms, g_next_wake_us - now)` when
  `g_next_wake_us!=0`, then calls `baud_drain_due()`. This is the "one-shot
  timer" in spirit — a precisely-timed wake, not a fixed-cadence poll —
  implemented as a cooperative sleep-duration computation rather than an
  async OS timer/signal, consistent with this codebase's existing avoidance
  of signal handlers touching scheduler state (see `HandleEvent()`'s
  poll-based Ctrl-C design).
- **`do_arbitrate()`** (`procstuff.c:1004`): one cheap check
  (`if (g_next_wake_us && now>=g_next_wake_us) baud_drain_due();`) at the top,
  covering busy (non-idle) periods where syscalls keep the scheduler moving
  without ever reaching `DoWait()`'s idle branch. Zero cost when
  `g_next_wake_us==0`.

`ConsPutc`'s only side effect is `write(1,&c,1)` (plus `gLastwritten_pid`
bookkeeping and the PTY branch for `gConsoleID>=TTY_Base`) — confirmed by
reading its current implementation — so calling it from the consumer instead
of inline in the producer's loop is safe; no hidden screen-buffer state
depends on synchronous timing. `gLastwritten_pid` should be set at *push*
time (in `fifo_push`, capturing the writer's pid) rather than at pop time,
since `currentpid` may have changed by the time a queued byte is actually
displayed.

## Interrupt handling (Ctrl-C / Ctrl-E)

`KeyToBuffer` (`utilstuff.c:911`) already special-cases the interrupt
(`pd_int`) and abort (`pd_qut`) characters to call `send_signal()`. Both
branches are extended to also flush that key's device's `baud_device_t`
(`head=tail=count=0`, `g_next_wake_us` recomputed) — `mco->spP` is already
available there and identifies the device. Input and output backlogs for a
given console reset together on interrupt/abort, matching real
terminal-break behavior: hitting Ctrl-C/Ctrl-E stops both what you were about
to receive and what's still queued to be shown to you.

**Normal process exit** needs no special handling at all: its queued output
just drains normally at baud speed after the process is gone, same as real
serial hardware finishing a transmission after its source disappears. This
is the simplest case — nothing to implement.

**Process kill** (`k <pid>`, abnormal termination) should *ideally* also
flush that process's device, mirroring `pCclose`'s existing
`gConsoleID= spP->term_id` resolution against the killed process's open
paths — but this is a best-effort nice-to-have, not a hard requirement. If
the kill path doesn't have a clean, low-effort way to resolve "which
device(s) was this process writing to," it's fine to skip for v1: the
backlog will still get cleared the next time the user hits Ctrl-C/Ctrl-E on
that console, or will simply finish draining on its own.

## Dispatcher changes (`os9exec_nt.c`, `procstuff.c`)

New `pstate_typ` value `pWaitWrite` (`os9exec_nt.h`'s `pstate_typ` enum).
Two narrow extensions from `==pWaitRead` to `==pWaitRead || ==pWaitWrite`:

1. The trap resume-and-retry block (`os9exec_nt.c` ~line 2022): restore
   saved registers/vector/func and re-run the same trap unchanged. This is
   generic (not read-specific) — it's what makes the *same* original write
   syscall actually re-execute with the same original buffer/length once the
   process wakes.
2. The scheduling-fairness check that gives a blocked process a timely
   re-check (`procstuff.c` ~line 1034, ~1198) — same treatment `pWaitRead`
   already gets.

Read-specific logic (`pwr_brk`, the `wRead` signal-delay check in
`send_signal`, `notW` in `filestuff.c`) is **not** touched or extended — this
is the specific thing that broke the earlier reverted attempt, and it simply
doesn't apply to a blocked writer.

State-name/display tables (`procstuff.c`'s `PStateStr`, `utilstuff.c`'s
DExec status mapping) get a `pWaitWrite` case added, cosmetic only.

## Testing

- Existing integration suite (`make test`, 88/0) must stay green — in
  particular `dump: shows hex`, which already exercises a large (20KB)
  buffered write through `-r`, is the regression check that the `-r`
  synchronous-drain path works correctly.
- New manual verification via `tools/os9repl.sh` (per project convention,
  Claude's own testing only): set a low baud rate with `tmode`, run a
  multi-line command, confirm output visibly trickles rather than bursts.
- Verify Ctrl-C/Ctrl-E during a slow-baud output burst: confirm the queued
  backlog stops immediately (flushed) rather than continuing to trickle out
  after the interrupt.
- Verify a write larger than 256 bytes at a slow baud rate correctly resumes
  after `pWaitWrite` blocking (no dropped/duplicated/reordered bytes) —
  compare full output against the `-r` (unpaced) rendering of the same
  command for byte-for-byte equality.
- Verify the `pid==129` sentinel path (system/banner output) doesn't crash
  or corrupt state — this was the specific landmine in the prior attempt.
