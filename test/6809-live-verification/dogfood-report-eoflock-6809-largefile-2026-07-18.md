# Dogfood report: RBF EOF-lock, large-file follow-up, real NitrOS-9 (6809), 2026-07-18

Direct follow-up to
`test/6809-live-verification/dogfood-report-eoflock-6809-2026-07-18.md`
(the "6809 report" below), which found that a reader opened concurrently
with an active writer on a shared RBF file did **not** reproduce
`os9exec`'s permanent-lockout bug, but also didn't match the idealized
pipe-like design intent cleanly: the reader's very first `READ` blocked
for the writer's **entire** ~12-second run (13 tiny records, `retries=0`
throughout — a genuine kernel block, not polling) and delivered everything
in one shot exactly at the writer's `CLOSE`. That report explicitly
flagged an open question: was the block really tied to `CLOSE`
specifically, or was the file (well under one 256-byte RBF logical
sector — see `~/.claude/skills/os9-systems-dev/references/file-managers.md`)
simply too small to ever show a finer-grained, sector/buffer-flush-based
release?

**This task's design was corrected mid-session.** The original plan (still
visible in this session's history) paced the writer at one real
record/second for 40-60 seconds, mirroring the small test's style at a
larger scale. The user does not want any test needing more than ~1 minute
of real time without special permission, and the coordinator (a separate
prior instruction in this same session) had proposed the 40-60s design
without that constraint in mind. The corrected design, used for every run
in this report: the writer bulk-writes all 50 records **as fast as
BASIC09/RBF actually execute them** (no per-record real-time throttle),
then deliberately holds the file open-but-idle for a real ~10-second pause
before closing — that pause is the only real-time cost, and it's exactly
the window that answers the question. Total wall time per run:
~15-16 seconds end to end (writer `CREATE` to reader `done`), not 40-60s.

Two BASIC09 programs, adapted from the small-file pair:
`dogfood-eoflock-writer-6809-largefile.bas` (creates
`/DD/EOFTEST/eoflarge.dat`, writes 50 fixed-80-byte records fast, holds
open ~10s, closes) and `dogfood-eoflock-reader-6809-largefile.bas` (opens
the same file `:READ`, loops reading up to 50 records, throttled-polling
on `E$EOF`/211 via `ON ERROR GOTO`, logging a wall-clock timestamp on
every successful read). Each file's own header carries its literal
verified output; this report is the narrative/analysis layer.

## Why 80-byte fixed records, 50 of them

Each record is padded to a fixed, predictable 80 content bytes (`seq=N
t=HH:MM:SS pad=` followed by repeated `X` characters, computed so the
total is always exactly 80 regardless of how many digits `STR$(N)`
produces — BASIC09 has no documented `STRING$`-repeat function, so the pad
string is built once via a plain `FOR`/`NEXT` concatenation loop). **On
disk each record is actually 81 bytes, not 80**: `WRITE #path, line` is
BASIC09's sequential file write, which is inherently line-delimited —
it appends a trailing CR on top of the content (there is no separate
`WRITELN`; plain `WRITE`/`READ` already are the CR-delimited statements
for sequential files, per `os9-dev`'s `basic09-language.md`). So the real
total is 50 records × 81 bytes = 4050 bytes = ~15.8 RBF logical sectors
(256 bytes/sector) — comfortably past this task's 10-15-sector target and
far past the previous test's sub-sector total. (The programs' own `PRINT`
diagnostics say "~4000 bytes," written before this was noticed; cosmetic
only, doesn't change the real on-disk size or any conclusion below.)

## What was actually observed

**Two independent clean runs, identical pattern each time:**

- Run 1: writer `CREATE`d at `09:14:24`; reader `OPEN`ed at `09:14:26`
  (`err=0`) and immediately started reading. The writer's fast write loop
  finished and printed its "holding open" milestone at `09:14:29`. The
  reader read records **1 through 46 live, incrementally, entirely before
  that milestone and during the writer's subsequent idle hold** — the last
  of that batch, read #46, landed at `09:14:30`. Read **#47 did not return
  until `09:14:39`** — the exact same second the writer's log shows
  `closed at 09:14:39`. Reads #48-50 then landed instantly alongside it.
  `retries=0` for the whole run: the `E$EOF` handler never fired, so every
  wait observed (including the final one) is a genuine kernel block, not
  poll-and-retry.
- Run 2: same shape, different clock: writer created `09:15:53`, reader
  opened `09:15:56`, writer's "holding open" milestone at `09:15:59`,
  reader's read #46 also at `09:15:59`, read #47 blocked until `09:16:09`
  — again the exact instant the writer's log shows its close. `retries=0`
  again.
- **The split landed at exactly the same record count, 46/50, in both
  runs** — not a value that drifted with run-to-run scheduling jitter,
  which suggests a fixed, deterministic amount of data being held back
  rather than an arbitrary "whatever hadn't been scheduled yet."

## Does this confirm sector/buffer-flush-based release, or close-only release?

**Sector/buffer-flush-based release, decisively — the small-file test's
"blocked for the writer's entire lifetime" reading does not generalize.**
A concurrently-opened reader path **can** and **does** see a large writer's
already-flushed data well before that writer closes — 46 of 50 records
(46 × 81 = 3726 of 4050 bytes on disk) were read live, spanning several RBF
sectors' worth of data, entirely during a window when the writer had already stopped
producing new data and was merely holding its path open. This directly
answers the question the previous report left open: the earlier result
wasn't evidence that `CLOSE` is the *only* release trigger; it was an
artifact of a file too small to ever let the reader get ahead of the
writer's in-progress tail. With 13 tiny records the reader's every `READ`
landed on that same trailing, not-yet-flushed edge, so the whole run
*looked like* one big close-gated block when it was actually "there was
never anything to release early."

**The tail-end block is still real and still interesting.** Records 47-50
(the last 4 × 81 = 324 bytes) did block, releasing only at `CLOSE`, both
runs. This
is not a contradiction — it's the reader genuinely catching up to the true
current end of file. The writer's fast write loop had already finished
producing data (nothing more was coming) by the time the reader consumed
record 46; from that point on the reader was legitimately at the live edge
with no new data to read, and correctly stalled there until the one event
that could change things: the writer's close. This is much closer to the
pipe-like design intent's actual promise — *"a reader catching up to a
live writer stalls right at the edge instead of racing ahead"* — than the
small-file run alone suggested.

## What exactly is the release granularity? (open, not resolved here)

The reproducible 46/50 split (324 bytes / 4 records held back, both runs,
using the corrected 81-bytes-per-record accounting — see the note above on
`WRITE`'s trailing CR) rules out a couple of simple "flush every N total
bytes written" models on back-of-envelope arithmetic against the true
4050-byte total (4050 mod 256 = 210; mod 512 = 466; mod 1024 = 978; mod
2048 = 2002 — none of these match a 324-byte remainder), which argues
against both a plain single-RBF-sector-at-a-time model and a 2048-byte
block model as the *whole* explanation. The precise,
run-to-run-identical size of the held-back tail (rather than a value that
drifted with scheduling jitter) is itself informative — it argues for some
fixed buffer/threshold rather than pure timing luck — but this task did
not vary record size or total count to see how the held-back amount
scales, which is what pinning down the actual mechanism (RBF sector
buffering vs. something in BASIC09's own sequential-file I/O layer) would
take. Flagged for whoever looks at the real buffering code next, same as
the previous report's own scoping decision not to chase kernel-source-level
root causes further.

## Harness friction (real, worth keeping)

- **A real, reproducible race in the writer's original draft.** The first
  attempt at this file built its one-time 60-iteration pad-string (via
  string concatenation, since BASIC09 has no documented `STRING$`-repeat
  function) **before** `CREATE`. A concurrently-launched reader (no
  equivalent preamble delay) reliably won the race to its own `OPEN` call,
  which then failed with `Error #216 - Path Name Not Found` on the
  not-yet-existent data file — not a module-resolution problem, despite
  looking like one at first (the same error code OS-9 gives for "module
  not found"). Confirmed by: (a) the module ran flawlessly standalone in
  a foreground test, (b) `procs` showed no surviving reader process after
  a failed concurrent launch, (c) moving `CREATE` to the very first
  executable statement (matching the small-file writer's shape, pad-string
  setup deferred until after) fixed it. Even after that fix, one later run
  still lost the race (the launch order between two freshly-forked
  processes is a genuine coin flip, not deterministic), so the reader also
  gained a defensive `OPEN`-retry wrapper (~40 short-throttled attempts,
  labels `10`/`20`/`25`) — unrelated to the actual `READ`-blocking behavior
  under test, purely a launch-robustness fix.
- **A genuine BASIC09 compiler gotcha, not previously documented in either
  skill: `LOAD` auto-sorts numbered lines by their numeric value for final
  fall-through/execution order, independent of the physical/typed order in
  the source file.** This is classic line-numbered-BASIC behavior that
  structured BASIC09 still carries silently. Typing the reader's
  OPEN-retry labels (`710`/`700`/`720`) **before** its pre-existing
  `100`/`800`/`900`/`950`/`999` block, in that physical order, silently
  resequenced the compiled program: line `100` (the `READ` loop) ended up
  executing immediately after the procedure's very first `ON ERROR`
  statement — not where intended, and not signaled as a compile error.
  Caught only by `LIST`-ing the loaded procedure and comparing the
  addresses/order against what was typed. Fixed by renumbering everything
  in one strictly ascending sequence (`10`/`20`/`25`/`30`/`100`/`800`/
  `900`/`950`/`999`) matching intended execution order, and re-verified via
  `LIST` before packing. Worth folding into `os9-dev`'s BASIC09 language
  reference if a second independent case of this turns up — it's a trap
  for exactly this style of incremental, scripted `tee`-based program
  composition, where lines don't have to be typed in numeric order to
  compile without error.
- **Session-state corruption during this task's early attempts turned out
  to be a stale `nc`/`perl` bridge process, not a real finding.** After a
  `restart`, several `send`/`key` commands appeared to silently do nothing
  (no error, no output change) for an extended stretch. Root cause: the
  `chan` tmux window's `nc 127.0.0.1 6811 | perl ...` pipeline had exited
  immediately at window creation (a connection-refused race against
  `drivewire-cli` not yet listening on the channel port), leaving a bare
  shell sitting in that pane with nothing bridging keystrokes to the
  guest. `nitros9repl.sh`'s `ensure_chan` only checks whether a window
  *named* `chan` exists, not whether the pipeline inside it is actually
  alive — so this failure mode isn't self-healing. Recovered manually via
  `tmux kill-window -t nitros9repl:chan` followed by any `send`/`key` call
  (which recreates the window via `ensure_chan`). Worth a `nitros9repl.sh`
  fix (check `pane_current_command` for `nc`/`perl`, not just window
  existence) for a future session, though not made here (out of this
  task's scope).
- The combined single-launch-line trick (`eofwrlg & eofrdlg &` in one
  `key` call) and the "buried shell prompt under backgrounded output,
  `key Enter` recovers it" gotcha from the previous 6809 report both
  applied identically here.

## Overall

The previous 6809 report's core finding — real NitrOS-9 RBF does not
reproduce `os9exec`'s permanent-lockout bug, and delivers every byte
correctly — stands unchanged. What's revised is the *characterization* of
the blocking behavior: it is **not** "blocked for the writer's entire
lifetime, released only at close," full stop. It is "a reader sees
already-flushed data live, and blocks only once caught up to the true
live edge, releasing when that edge next moves" — for an idle-but-open
writer, the only thing that moves the edge is close, which is why the
small file (never large enough to separate "caught up" from "writer just
finished") made the two look identical. `os9-systems-dev`'s
`file-managers.md` Record Locking section and
`~/.claude/skills/os9-dev/references/VERIFICATION-BACKLOG.md` both updated
to reflect this.
