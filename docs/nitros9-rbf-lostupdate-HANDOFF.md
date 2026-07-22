# NitrOS-9 6809 RBF lost-update — RESOLVED (2026-07-21, session 3)

**Root cause found, fixed, and A/B/A-proven.** The record-lock retry path
(`L0B11`→`L0B1D`) reloaded only A from the `L0B1B` frame; B carried `L1053`'s
`P$Signal` read (0 after a consumed `S$Wake`) and X the `F$Sleep`/`PE.TmOut`
leftover (0). Any request under 256 bytes re-presented after a park as the
count-0 "dismiss" request → `L0B9F` released everything and returned
carry-clear → the woken waiter ran its GET/PUT with **no lock at all**. That
one defect produces every observation below: the same-stamp collision pairs,
the two-buffer divergence windows (unlocked concurrency reopens the
SINBUF-set-after-load window), "no both-hold" (the degenerate path never sets
RcdLock), the poll fix's accidental success (its retry's B = `comb`'d owner ID
≠ 0 → real overlapping claim), and wakedelay's failure (park kept → retry
still degenerate).

Fix: `docs/nitros9-rbf-lostupdate-regfix.patch` (+4 bytes: `lda ,s` →
`ldd ,s` + `ldx 2,s`, one `bne L0B11` → `lbne`). Wait chain, S$Wake handoff,
`PE.TmOut`, and deadlock detection all preserved — verified live
(holder/waiter blocks 3.4s and reads post-write; crossed two-path holds give
exactly one `E$DeadLk` #254 on fix AND stock; rl-race3o log shows 400/400
distinct reads). Losses/400: stock 21/59/21/48/64 → fix 0/0/0/0 → stock
110/73 → fix 0. Reference modules + full story:
`docs/nitros9-rbf-reference/README.md`. Observation tooling that cracked it
(all in `test/6809-live-verification/`): `rl-race3o.bas`/`rl-dump.bas`
(in-memory GET logs, writer stamps inside the record), `rl-chk2.bas`,
`rl-dead1/2.bas` (two-path deadlock pair), `rl-racecell.sh` (one guarded
measurement), `rl-obs-analyze.py` (duplicate/window analysis of the logs),
`rl-obs-disksample.py` (host-side flush detection — proved the race is
all-RAM). The polling stopgap `nitros9-rbf-lostupdate-FIX.patch` is
OBSOLETE — do not use it.

Everything below is the pre-resolution investigation record, kept for
history; its "mechanism NOT cleanly cracked" framing is superseded.

---

# (superseded) root cause CONFIRMED (2026-07-21, session 2)

Supersedes the SEEK/reacquisition-gap framing of the previous handoff. Those
hypotheses are **dead** (see "Ruled out"). The mechanism is now localized by
experiment to the **simultaneous first sector LOAD of a shared file**, not the
record lock, not the conflict ring, not `SEEK`.

## 2026-07-21 session 2 — STATE: working (polling) fix; mechanism NOT cleanly cracked

Read this first. Clean, trusted facts (no perturbing instrumentation):
- Lock **detection works** (E\$Lock probe: conflicts are seen every run).
- Guest-side **detect+retry loses nothing** (`rl-race3e` on the probe = 0).
- **No both-hold**: a two-sided detector (both racers check, after claiming, whether
  the sibling also holds RcdLock) NEVER fires -> the claim serializes; two paths
  never hold the record at once.
- **NOT eof-specific**: `rl-init2.bas` makes rl.dat 20 bytes so record 0 is not at
  end-of-file; the SAME racer on STOCK base STILL loses (81, 86, ...).  So the
  earlier "GET holds EofLock not RcdLock (#250)" reading was an ARTIFACT of a
  write-path probe that also emits spurious #211 read errors -- do not trust the
  write-entry probes (`rbf.wrchk`/`rbf.anylk`); they perturb the path.
- **De-sync fixes it; nothing else does**: the `rbf.poll.mn` fix (short sleep +
  recheck instead of the wait-chain S\$Wake sleep) = 0, A/B/A.  Masking the claim,
  buffer-invalidate-on-wake, keep-wait-chain+delay: all still lose.

The contradiction that is still unresolved: the claim serializes (no both-hold),
detection works, it isn't eof-specific -- yet the read-modify-write loses, and ONLY
de-synchronising the woken waiter's reacquire fixes it.  The most consistent
remaining reading is that the update read's lock is not effectively in force across
GET->PUT for a timing-dependent subset, which de-sync avoids -- but I could not
pin the instruction, and the write-entry probes that would show it are unreliable.
A cleaner (non-perturbing) way to observe the lock state at PUT time is the next
step.  The working poll fix below is real and validated; the clean root-cause and
the proper (non-polling, deadlock-preserving) fix are still open.

## 2026-07-21 session 2 — working fix (polling), A/B/A-validated

**The lost update is the woken waiter reacquiring the record in lockstep with
the releaser.** When the holder's WRITE releases the record it sends `S$Wake`;
the woken waiter (`L0B42`→`L0B11`) reacquires *synchronized with* that release,
and racer + releaser both rush the just-freed record. De-synchronizing that
reacquire eliminates the loss entirely.

**Fix (proven): `rbf.poll.mn`** — one hunk. At `L0B1D`, right after
`bsr L0B9F / bcc L0B9D`, a detected conflict does `ldx #1 / os9 F\$Sleep /
lbra L0B11` (brief bounded sleep, then recheck) instead of the wait-chain
sleep-until-woken. **Concurrent racer, A/B/A on the test disk:**

| phase | losses (5-10 runs) |
|---|---|
| A  base  | 4, 6, 87, 85, 8 |
| B  poll  | 0, 0, 0, 0, 0  (and a separate 10/10 all 0) |
| A' base  | 61, 7, 95, 15, ... |

Zero across every poll run; losses return on revert. (Also independently
proven: the same racer with the E\$Lock **probe** RBF + a guest retry loop =
0 — guest-side de-sync loses nothing either.)

**It's the wait-chain sleep itself, not just reacquire timing.** A refined
variant `rbf.wakedelay.mn` that KEEPS the wait-chain sleep-until-woken and only
adds the 1-tick de-sync at `L0B11` (reacquire) **still loses** (91,14,9,85,37,
105,3,58,9,121). Only the poll — which bypasses the wait-chain sleep entirely
(never links into `PE.Wait`, never woken by `S$Wake`, just short-sleep +
recheck) — works. So the defect lives in `L0B42`'s wait-chain-sleep + the
`L0AD1` `S$Wake` reacquire path, and the correct fix must AVOID that sleep, not
delay after it.

**Caveats / for the owner to refine:** the minimal poll hunk (a) polls at 1-tick
granularity instead of sleeping until woken, and (b) as written drops the
deadlock/`PE.TmOut` timeout path (a real deadlock would poll forever). I tried a
production variant `rbf.poll2.mn` that keeps `L0B30` (deadlock detect) + the
`P$DeadLk` wait-marking and replaces only the `L0B42` sleep with the poll — **it
HANGS at login** (wedged before any racer ran), so integrating the poll while
keeping the wait-marking/deadlock machinery is not a clean drop-in and needs
your hand. Recommended path: keep the deadlock CHECK (`L0B30`) but resolve the
wait with a bounded poll (respect `PE.TmOut` as a poll count, then `E$Lock`) and
be careful about `P$DeadLk` set/clear ordering — that's where poll2 tripped.
The operative fix is confirmed and A/B/A-proven: *do not resolve a record
conflict via the wait-chain `S$Wake` sleep; use bounded poll + fresh recheck.*
The one-hunk `rbf.poll.mn` is correct for the lost update and safe for boot/login
(verified); it just trades the deadlock/timeout niceties, which you can restore.

**Deeper note (unresolved curiosity):** de-sync fixes it *even with base's
non-atomic claim*, while masking the claim atomic (`fixlockonly`) did *not* fix
it *with* the synchronized wake. So the operative variable is the wake timing,
not claim atomicity — consistent with the woken waiter reading a value the
release left stale until the next migration catches up, which the 1-tick delay
allows. Worth your eye.

## (superseded framing) SOLVED to a proven ~40-line region: RBF's own sleep/wake

**The lock's DETECTION is complete and correct. The entire lost update is in
RBF's internal sleep/wake/reacquire.** Proven by a two-part experiment:

- **Probe RBF** (`rbf.probe.mn` = stock + one change: at `L0B1D`, right after
  `bcc L0B9D`, `ldb #E$Lock / bra L0B9A` — a detected conflict returns Error 252
  instead of sleeping). Run the normal racer: **both racers hit
  "Error #252 - Record is locked-out" every run.** So the first GET locks and
  the second GET notices — reliably.
- **Probe RBF + `rl-race3e`** (racer that retries the GET on Error 252, yielding
  a tick each try — guest-side serialization instead of RBF's sleep/wake):
  **lost = 0**, ~20 retries/racer/run. Same detection, same conflicts, same
  timing — resolved in the guest, nothing lost.

So: detection ✓, guest-side resolve ✓. **When RBF resolves the conflict itself —
sleep at `L0B42`, wake via `L0AD1` on the holder's release, reacquire via
`L0B11` — updates are lost.** The guest works because each retry is a FRESH
conflict-check that can never proceed while a conflict exists; RBF's wake path
reacquires-in-place and something in that sleep→wake→reacquire loses the update
(reacquire race with the holder's next op through the release→wake window, or a
woken waiter proceeding on stale state — not yet pinned to the instruction).

**Fix target (finally precise):** `rbf.asm` `L0B42`..`L0B9D` (sleep + unlink +
reacquire) and `L0AD1` (release-and-wake). NOT the claim (masking it =
`fixlockonly` = no effect), NOT the buffer (`loadfix2` single-buffer = no
effect), NOT the ring, NOT the open. Six such fixes were built and measured to
no effect — do not repeat them; the guest-retry proof says the fault is
strictly in the internal sleep/wake.

**For the owner (you designed this sleep/wake record lock):** what should a
woken waiter do to be safe against the holder reacquiring in the same window?
The guest-retry that works does "detect → back off → redo the whole GET from
scratch"; RBF does "detect → sleep in place → reacquire → continue the read."
The delta between those two is the bug.

New fixtures: `rl-race3e.bas` (retry-on-E\$Lock racer). Probe recipe above is
trivial to rebuild. `rbf.probe.mn`/`rbf.fixlockonly.mn`/`rbf.loadfix2.mn` were
in `/private/tmp/rlwip/` during the run.

## (earlier) session 2 addendum — it's the LOCK, and there's a paradox

Later work overturned the "buffer-load race" emphasis below. A fix that
GUARANTEES a single shared sector buffer (atomic buffer-claim + reserve-before-
read, `rbf.loadfix2`, built + login-clean) **still loses** at baseline levels.
With one shared buffer, two GETs losing an update can ONLY mean they both read
the same value — i.e. **the record lock is not serializing two simultaneous
same-record update-mode GETs.** The buffer was downstream.

**The paradox for the owner:**
- The auto-lock demonstrably BLOCKS a second update-mode GET: `rlhold2` holds,
  `rlwait`'s GET blocks ~60s and reads the post-write value. So the lock works
  when one path leads.
- Yet two rapid racers lose (both read the same value ⇒ not serialized).
- Making the lock CLAIM atomic (mask the conflict-walk-and-set — exactly what the
  uncommitted working-tree FIX does at `L0C01`) does **NOT** fix it (`wip`).
- Making the buffer single-copy does **NOT** fix it (`loadfix2`).
- Staggering the two GETs by 1.5s DOES fix it (~0), and it's the GET timing, not
  the open (delaying only the first GET, `rl-race3g`, is as good as delaying the
  whole open).

So what serializes two rapid same-record update-mode GETs in this design, if not
the masked record-lock claim? Base's claim is a check-then-set split across two
routines (`L0BC2`/`L0C01` walks and returns carry; `L0BAA` then sets `RcdLock`)
with a `L0AD1` WAKE (a syscall) in between — unmaskable as-is. The FIX moved the
set into the masked walk and still failed, which is the part I can't explain.
Six fixes total ruled out (see list). This is where I need your read.

## The one-paragraph answer (earlier framing — buffer emphasis now demoted)

Two update-mode paths open the SAME one-record file and hammer it
(`SEEK 0 / GET / +1 / SEEK 0 / PUT`, 200x each). When their **first GETs load
sector 0 at the same time**, each path reads its **own private copy** of the
sector, because RBF's buffer-sharing walk only recognizes a sibling that has
already finished loading (`SINBUF` set). The sector loader sets `SINBUF`
**after** the disk read (`L12B5`), so during a concurrent load neither path is
visible to the other and two independent buffers form. Two camped copies never
re-converge, so one path's `PUT` overwrites the other's — the lost update.
Once the sector is a single shared buffer (staggered load), the record lock and
buffer migration work perfectly and **no update is lost**.

## Decisive evidence (all on the test disk, base RBF unless noted)

- **Staggering the OPEN fixes it.** `rlrace3d` delays its OPEN 1.5s; raced vs a
  plain `rlrace3`: 0,0,0,3,0 losses vs the concurrent 17,3,102,62,30.
- **Staggering only the first GET fixes it too.** `rlrace3g` opens
  concurrently but delays its first GET 1.5s: 0,5,5,2,0. So the trigger is the
  **load timing**, not the open — delaying just the load helps as much as
  delaying the whole open. (Small residual ~2-5 = a secondary migration/lock
  race during rapid RMW; a correct load fix should also close it, since the
  same two-copy race recurs whenever both paths reload simultaneously.)
- **The record lock is NOT broken.** Holder+waiter with a head start:
  waiter blocks and reads the post-write value (count=1) every trial. The ring
  is built correctly and locking engages whenever one path leads.
- **Concurrent-open loss is huge-variance**, 0-110 per 400. The old handoff's
  "332/400 stable" was one sample; there is no stable number.

## The exact code

- Loader `L12B5` (rbf.asm ~3482): `lbsr L113A` (disk read) THEN
  `oim #(BufBusy!SINBUF),PD.SMF,y`. `SINBUF` is set only after the read.
- Buffer-share walk `L126B` (~3447): matches a sibling on the same sector, then
  `bita #SINBUF ; beq L126B` — it **skips** a sibling that is mid-load
  (`InDriver` set, `SINBUF` not yet). So two simultaneous loaders miss each
  other. `V.BUSY` serializes the actual driver reads but the second loader,
  already past the walk, reads its own copy anyway rather than adopting the
  first's.

## THE DESIGN QUESTION FOR THE OWNER

In the original Microware RBF, when a path loads a sector, was the buffer
marked as **holding/reserving that sector BEFORE the physical read** (so a
concurrent loader of the same sector would wait and then share the one buffer),
or only after? NitrOS-9 marks it after (`SINBUF` post-read), which opens the
two-copy window. If the original reserved-before-read, this is a clone
divergence and the fix is to restore that ordering. The subtlety is the wake:
the loaded/valid signal must become visible **before `V.BUSY` is released**, or
a queued loader wakes into a not-yet-valid state. That ordering constraint is
why the naive fixes below fail, and why this needs your call rather than another
blind patch.

## Ruled out — do NOT re-attempt these (each built + measured this session)

1. **`WritPub`** (prior session's uncommitted buffer-invalidate-on-write, still
   in the working tree): no effect. It invalidates a stale sibling on WRITE, but
   the two copies form at LOAD, and the invalidate races the sibling's read.
2. **Masking `L0A2A`** (conflict-ring insert): no effect — results statistically
   identical to base. The ring is not the problem (holder+waiter proves it).
3. **Wait-on-loader in `L126B`** (make the walk wait on an `InDriver` sibling of
   the same sector): no effect. `InDriver` is set only during the driver call,
   too narrow — the walker usually misses it.
4. **`FIX + L0A2A mask` (wip2) HANGS on login** — the added open-path mask
   nests badly with the FIX's record-lock-claim mask. A caution for any masking
   fix in the open/load path: verify login still works before measuring.

Reference modules built this session live in `/private/tmp/rlwip/` during the
run (gone on reboot); rebuild from source with the `lwasm` line the recipe
emits (`make -n .mods/rbf.mn` under `source/recipes/coco3/dw`, `H6309=0`).

## Fixtures (all in test/6809-live-verification/, uploaded via nitros9-upload-b09.sh)

- `rl-init.bas` (rlinit), `rl-race3.bas` (rlrace3), `rl-check.bas` (rlcheck) —
  the core self-verifying racer (rec(5)=2 ⇒ both finished; 400-rec(1)=lost).
- `rl-race3d.bas` (rlrace3d) — delays OPEN 1.5s (stagger-open control).
- `rl-race3g.bas` (rlrace3g) — delays first GET 1.5s (stagger-load control).
- `rl-qhold.bas` (rlqhold) + `rl-wait.bas` (rlwait) — holder/waiter ring test.
- `rl-race3h.bas` (rlrace3h) — wide GET->PUT hold; NOTE it can hang the REPL
  under contention (a waiter parking on a lost wake); prefer the racers above.

Run shape: `del rl.dat; rlinit; rlrace3 >>>bg&; rlrace3`; wait; `rlcheck`.
Environment: `docs/nitros9-rbf-lock-HANDOFF.md` cold-start recipe (private image
copy, private tmux/ports, install a reference RBF module). Use
`NITROS9REPL_DISKDIR` pointed at a private disk dir — the `.ide` is the only
thing the guest writes.

## Posture reminder (owner's standing rules, all honored here)

Reproduced before believing; A/B/A'd every module; exonerated the harness (solo
racer control was queued; the lock/ring were cleared by direct test before RBF
was blamed). The remaining step is a design decision, not more measurement.
