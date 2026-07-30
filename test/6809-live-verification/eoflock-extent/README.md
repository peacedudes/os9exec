# How far does the EOF lock actually reach? (6809, live NitrOS-9)

Measured 2026-07-29/30 against NitrOS-9 V3.3.0 on the EOU disk, to settle a
question §6.6.3 of the *OS-9 System Programmers Manual* raises but does not
answer: a writer gains EOF Lock, and "will keep the end of file locked out
until a read or write is performed that is not at the end of the file" — but
how much of the file is "the end", and what exactly drops it?

The prompt for this was a specific doubt, and it was worth testing rather than
assuming: once A has written at the old EOF, that byte range holds A's own
record and EOF has moved past it, so a second writer aiming at the *old* offset
is arguably no longer writing "at the end" and might sail straight through.

It does not. Results, one fresh boot per case:

| contender's write | while A holds the lock | outcome |
|---|---|---|
| offset 0 — far ahead of A, plainly not the end | outside the range | **not blocked**, returned in 1s |
| offset 10 — A's own just-written record, the old EOF | inside the range | **blocked** until A released |
| offset 20 — the new true EOF, past A's record | inside the range | **blocked** until A released |
| release: A performs a write at offset 0 | — | **drops the lock there**, contender proceeds *before* A closes |

So the lock covers from where A wrote to the end of the file — not the whole
file, and not merely the single byte range past A's record — and it is dropped
by A's first not-at-the-end access, exactly as §6.6.5 states, rather than being
held until close.

## The fixtures

BASIC09 *session* transcripts (`e`/`q`/`run`/`bye`), same form as
`../gfx2-scripts/`, not `.bas` procedures. A record is `rec(5):INTEGER` = 10
bytes, so offset 10 is the end after one seed record — the same arithmetic
`t11wprod` in the conformance suite uses to reach EOF.

- `einit.s` — creates `eof.dat` with one record, so EOF = 10
- `ehold.s` — A: writes at EOF(10), then spins holding the path open
- `ehold2.s` — A variant that performs a non-EOF write partway through, to
  time the release rather than infer it from the close
- `eover.s` / `eozero.s` / `eoend.s` — B, writing at offset 10 / 0 / 20
- `edump.s` — reads the records back
- `run-case.sh` — drives one case end to end

## Two traps this cost real time to find

**Size the hold against emulator speed, not a real 6809.** `-no-ratelimit`
runs the CoCo3 hundreds of times faster than hardware, so `rl-hold.bas`-style
delays of 12,000–24,000 iterations finish before the contender has even
started, and the contender then "returns immediately" for a reason that has
nothing to do with locking. About 36M iterations gives a ~60s window here.
`run-case.sh` refuses to report a case whose holder never reached its write,
or had already released — a fixed sleep silently produced exactly that.

**One holder per boot.** Reusing a guest whose previous holder had been
Ctrl-E'd left a contender blocked for 950s+ with no way to tell which holder
was responsible. `run-case.sh` therefore boots fresh per case, so exactly one
holder exists by construction.
