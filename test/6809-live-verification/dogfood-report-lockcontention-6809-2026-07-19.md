# Dogfood report: decisive record-lock contention test, NitrOS-9 (6809) (2026-07-19)

Direct follow-up to the lost-update race
(`dogfood-report-lostupdate-6809-2026-07-19.md`), which proved NitrOS-9
loses updates under rapid repeated contention but never established
*whether contention ever actually blocks at all* — the same
"discriminating question" the designer used to establish that `os9exec`
has zero record locking at the source level (`SS_Lock` is a no-op
returning success). This test answers that question directly for
NitrOS-9.

Design, per the designer's own restated mechanism
([[user-designed-rbf-eof-lock]]): a `Read` in update mode should lock the
record it read; a process trying to `GET` the same record should sleep
until the holder's `Write` releases it, then wake with the released
data. Two BASIC09 programs:

- **Holder** (`dogfood-lockcontention-holder-6809.bas`): opens the shared
  record (`UPDATE`), `GET`s it (should acquire the lock), prints a
  marker, then deliberately burns real wall-clock time (a 60,000-iteration
  empty `FOR` loop) *without releasing the lock*, prints a second marker,
  then `PUT`s (releases) and closes.
- **Waiter** (`dogfood-lockcontention-waiter-6809.bas`): opens the same
  file (also `UPDATE`), prints a marker, then tries to `GET` the same
  record. If NitrOS-9 has a real lock, this call should not return until
  the holder's `PUT` releases it, and the count it reads back should be
  the holder's *new* value, not the stale one that existed when the lock
  was taken.

Launched concurrently (`lockholder & lockwaiter &`), two independent
races, counter reset between them.

## Result: NitrOS-9 has a real, functioning record lock for this pattern

**Race 1** (clean, unambiguous ordering):
```
waiter: about to GET
holder: locked record, count=0 -- holding
holder: about to release (PUT)
holder: released
waiter: GET returned, count=1
```
The waiter's `about to GET` marker printed *before* the holder even
finished locking the record, yet `GET returned` didn't print until
*after* the holder's entire hold-and-release sequence completed — and
the value it read back is the holder's post-release value (`1`), never
the pre-lock value (`0`) that existed at the moment the waiter attempted
its read.

**Race 2** (reproduced the same outcome, less clean ordering evidence but
identical conclusion):
```
holder: locked record, count=0 -- holding
holder: about to release (PUT)
waiter: about to GET
waiter: GET returned, count=1
holder: released
```
Here the waiter didn't reach its `GET` call until the holder was already
mid-release, so this run alone doesn't prove blocking as cleanly — but
combined with race 1, the pattern (waiter's `GET` never returns with the
stale pre-lock value) is consistent across both trials.

**If NitrOS-9 had no real lock** (the way `os9exec` has been shown to
have none), the waiter's `GET` would be expected to return immediately
with `count=0` in at least some runs — a race with no synchronization
mechanism at all doesn't reliably produce "always see the other side's
final value." Getting `count=1` in both independent races, plus race 1's
unambiguous ordering proof, is solid evidence of real blocking, not
coincidence.

## Reconciling this with the lost-update finding

This appears to contradict `dogfood-report-lostupdate-6809-2026-07-19.md`
(457 and 584 lost updates out of 600), but it doesn't — it sharpens it.
**NitrOS-9's lock mechanism works correctly for a single, well-separated
lock/hold/release cycle** (this test) **but does not fully prevent lost
updates under rapid, tight-loop re-acquisition** (the earlier test, 300
back-to-back iterations with no delay between a `PUT` and the next
`SEEK`/`GET`). The likely explanation, not confirmed further here: a race
window in the release-then-reacquire sequence itself — if a released lock
briefly leaves the record unlocked before the *next* iteration's `GET`
re-acquires it, a competing process's `GET` landing in that narrow window
would succeed without waiting, producing exactly the intermittent,
variable-magnitude loss observed (143 lost in one race, only 16 in
another — consistent with a rare, timing-dependent window rather than a
mechanism that never locks at all).

**This means NitrOS-9 is not simply "broken" for record locking** — it
has a real, demonstrated locking mechanism, whose failure mode under
rapid contention is more specific (and more interesting) than "no lock
exists." Neither reimplementation should be treated as an oracle for the
original design either way (see [[user-designed-rbf-eof-lock]]'s
"NitrOS-9 is not a correct oracle" correction) — this is a description of
what NitrOS-9's own code actually does, not a claim about what the design
requires.

## Suggested next step, not done here

A version of the lost-update racer that inserts a tiny deliberate delay
between `PUT` and the next iteration's `SEEK`/`GET` (long enough to make
any release/reacquire race window observable, short enough to keep the
test under a minute) would help confirm or refute the race-window theory
directly — if lost updates disappear or drop sharply with the delay
added, that's strong support for a narrow reacquisition race; if losses
persist at the same rate, the mechanism has a different, deeper gap.

## Harness notes

- Same buried-shell-prompt gotcha as the lost-update pass: after both
  backgrounded racers finish and print, the next `send` can time out even
  though the shell is genuinely idle — `key Enter` recovers it.
- `lockinit`/`lockholder`/`lockwaiter` must each be `LOAD`ed *and PACKed*
  from inside `basic09` before they're reachable as bare shell commands —
  running `RUN <name>` inside BASIC09 alone doesn't make the name
  available outside that BASIC09 session; easy to forget when iterating
  and re-hit `Error #216`/`Error #043`.
