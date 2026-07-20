# Dogfood report: RBF EOF-lock design intent, OS-9/68000 (2026-07-18)

Task: check whether `os9exec`'s RBF reimplementation honors the original
design intent of the EOF-lock mechanism, as described firsthand by this
project's owner (who personally designed the original Microware
version): a slow writer (A) appending to a growing file and a slow
reader (B) consuming it should coordinate "as if it were a pipe" -- B
freely reads everything A has flushed, and blocks right at the current
write position until A's next write, rather than mistaking "caught up
to current EOF" for "writer is done." The task asked specifically
whether `os9exec` delivers that pipe-like behavior, or instead behaves
like the naive "B is locked out until A exits" reading -- these are
different, testable outcomes. Target: OS-9/68000 via `os9exec` and
`tools/os9repl.sh`, tested against a **real RBF disk image** (`/h1`,
mounted from `os9exec-git_code/os9/h1`) rather than a host-native
directory, since `using-os9exec-repl.md` notes host-native mounts have
no real record-locking machinery underneath -- this distinction turned
out to matter (see below).

Three BASIC09 programs, PACKed and run standalone via the shell's
bare-name RunB auto-detection, same pattern as the earlier pipes dogfood
pass:

- `dogfood-eoflock-writer.bas` (A) -- appends a timestamped record once
  per real second, 40 times, to `/h1/CLAUDETEST/eoflock.dat`.
- `dogfood-eoflock-reader-readonly.bas` (B, READ-only) -- opens the same
  file `:READ` and loops reading records, throttled-polling on E$EOF.
- `dogfood-eoflock-reader-update.bas` (B, UPDATE mode) -- identical
  shape, opens `:UPDATE` instead.

Each file's header carries its own literal verified output; this report
is the narrative/analysis layer.

## What was actually observed

**Neither of the two candidate outcomes the task set out to
distinguish was what happened.** The real result is a third, more
severe outcome that the task's framing didn't anticipate:

**A reader path (either READ-only or UPDATE mode) opened while a writer
path is concurrently open on the same file never sees a single byte of
that file's data, for the reader's entire remaining lifetime -- not
just "not yet," but never, even long after the writer finishes and
closes.** This held in every clean test:

- A reader opened essentially at the writer's very first WRITE (READ-only
  round, `eofwr & eofrdro &`, both launched in one shell command line):
  zero successful reads, out of 40 records the writer wrote to that
  exact file while the reader's path stayed open, and still zero 11
  seconds *after* the writer closed, until the reader's own retry cap
  (401 attempts) gave up.
- A reader opened when the writer already had **34 records flushed**
  (real backlog present, no startup race possible -- this was the
  original, non-combined-launch round): same result, zero successful
  reads, all the way through writer close and past it.
- The identical pattern reproduced for **both** READ-only and UPDATE
  mode in clean single-instance runs, with retries advancing in
  lockstep with the writer's once-per-second WRITEs (confirmed by
  cross-referencing each reader's retry timestamps against the writer's
  own concurrent `wrote seq=N` trace) but never once returning real
  data.
- A **brand-new OPEN issued after the writer has closed** reads the
  file perfectly: all 40 records, in order, correct content, zero
  retries, both in READ-only and UPDATE mode. The file and its data are
  completely fine -- this rules out data corruption or loss. The bug is
  specific to a path that was open *concurrently* with an active
  writer; that same path never recovers visibility into the file for
  the rest of its own lifetime, independent of the writer's later
  fate.

So, to answer the task's specific sub-questions:

1. **READ-only mode**: B's own reads are indeed unaffected by whatever
   locking A's writes are doing (per `file-managers.md`'s "reads on
   read-only paths never lock anything") -- but that turned out to be
   true in a much more damaging sense than intended: this reader
   doesn't merely skip *participating* in locking, it appears to never
   see the underlying file's growth *at all* while a writer path is
   open on it.
2. **UPDATE mode**: made no observable difference. Same total,
   permanent non-visibility, same lockstep-but-always-EOF retry
   pattern. (One messier, non-reproduced run showed a lucky duplicate
   UPDATE-mode instance reading a few records -- flagged as an
   artifact below, not a real capability difference.)
3. **Blocking vs. immediate-return**: clearly **immediate-return,
   non-blocking** at the syscall level in every clean run -- each failed
   READ returns E$EOF (211) right away and the program's own retry loop
   is what re-issues the call; this is not RBF holding the call open
   and returning it later with fresh data. (An earlier *unthrottled*
   version of the retry loop, before this was understood, produced
   100+ retries per real second from two accidentally-duplicated reader
   instances -- itself confirming immediate-return, since a real
   blocking wait could not produce that volume.)
4. **Does B ever unblock promptly on A's next write?** No -- in every
   clean run, B never unblocked at all, not promptly and not
   eventually. It stayed stuck at zero successful reads for the
   reader's whole run, including well past A's close. This is the
   opposite of "unblocks promptly, pipe-like" and also does not match
   "locked out only until A exits" (exiting didn't help either, for the
   already-open reader path).

## Does this match the original design intent?

**No.** It matches neither candidate reading the task asked about. The
design intent (owner's own words): B should freely consume everything
A has already flushed, and only stall right at the live edge. What was
observed instead: B sees **nothing A has flushed, ever, for the life of
B's own path** -- worse than "locked out until A exits," because A
exiting doesn't fix it either. Only closing B's path and opening a
*fresh* one recovers visibility. This looks like a genuine `os9exec`
RBF bug in how a second, concurrently-opened path perceives a growing
file's extent -- plausibly the shared/on-disk notion of "current file
size" that a `Read`/`ReadLn` call checks against isn't being kept in
sync with a different, already-open path's view once that view was
established while a writer held the file open, and nothing later
(including the writer's own close) triggers a re-check for that
specific already-open reader path. Per the task's instructions, no fix
was attempted here -- this is flagged for follow-up, not root-caused
further (the four-way non-determinism between "opens at file-creation
instant" and "opens with 34 records already flushed" behaving
identically rules out a simple race-at-creation explanation, but
doesn't pin down the actual mechanism).

## Was `file-managers.md`'s Record Locking section wrong, missing, or confusing?

- **Missing, and this is the main gap**: the section describes what
  locks a `Read`/`Write` *takes* and when they're *released*, but says
  nothing about what a reader actually experiences when it catches up
  to a writer's current EOF while another path holds the EOF lock --
  neither "blocks" nor "returns immediately, poll yourself" is stated
  either way. A reader relying on this doc alone would have no way to
  predict the (very consequential) difference between "poll in a loop"
  and "just issue one blocking READ and wait." This dogfood pass had to
  discover the actual mechanism (immediate-return, non-blocking) by
  direct experiment.
- **Not wrong, but incomplete given what was found**: "Reads on
  read-only or execute-mode paths never lock anything, since those
  modes can't update records anyway" is accurate as a statement about
  *locking*, but reads as reassuring ("read-only is simpler/safer") when
  live behavior shows read-only concurrent reads are actually **more**
  broken than the doc's framing would suggest -- not merely
  lock-indifferent, but apparently blind to the writer's data entirely.
  Worth a caveat once the underlying `os9exec` behavior is confirmed
  further or fixed.
- Everything else in the section (write-always-releases-first,
  zero-byte read/write dropping all locks, `SS_Lock`/`SS_Ticks`,
  `seek()` not affecting locking) wasn't specifically exercised by this
  pass and nothing encountered contradicted it.

## Harness friction

- **The single sharpest piece of friction**: sending a second `key`
  command (to launch B) while A's backgrounded job is actively
  streaming output to the same terminal is **unreliable** -- several
  attempts across this session were silently dropped (no echo, no
  effect, confirmed by full-pane `grep` turning up nothing), while
  others eventually landed several seconds late, and at least one
  landed but visibly character-interleaved mid-line with the writer's
  own output (`writer: wrote eofrdro &` -- the keystrokes got spliced
  into a line the writer was actively printing). **Fix that worked
  reliably**: send both launches as a single combined shell command
  line (`eofwr & eofrdro &`) in one `key` call, issued while the
  terminal is still idle -- both processes then start within the same
  real second and the send never races live output. Recommend adding
  this pattern to `using-os9exec-repl.md` for anyone launching two
  chatty concurrent background jobs.
- **A retried "did it land?" check caused real duplicate-process
  contamination** in two rounds of this session: after a `key` send
  produced no visible confirmation, retrying it (reasonably, given the
  unreliability above) sometimes turned out to have actually landed the
  *first* time too, launching two instances of the same reader
  concurrently. This produced hopelessly interleaved, hard-to-attribute
  terminal output (illustrated in `dogfood-eoflock-reader-update.bas`'s
  header) and is the likely explanation for the one non-reproduced
  "UPDATE mode partially worked" result mentioned above -- plausibly an
  artifact of two concurrent UPDATE paths interacting with each other's
  locks, not a real capability. The combined-single-line-launch fix
  above also avoids this failure mode, since there's only ever one
  `key` call to retry-or-not.
- **An unthrottled retry-poll loop is dangerous, not just noisy.** The
  first version of the reader scripts had no delay between EOF retries.
  With two accidentally-duplicated instances both spinning, this
  generated well over 1000 retries/second combined, and the
  concurrently-running writer appeared to stop making progress
  entirely for 60+ real seconds (its own once-per-second trace lines
  vanished from view and, more importantly, its record count stopped
  advancing) -- plausibly CPU starvation of the writer process by
  os9exec's scheduler under two tight, no-yield polling loops. This
  eventually required a full `./tools/os9repl.sh restart` to recover a
  usable terminal (session state lost, host-native files intact, exactly
  as documented for this class of recovery). This is a distinct finding
  from the locking question itself, flagged for whoever looks at
  scheduler fairness next -- a throttled retry loop (small `FOR/NEXT`
  busy-wait between retries, used in the final, archived versions of
  both reader files) avoided a repeat of this in every later run.
- **No Ctrl-E-specific finding this time** -- unlike the earlier
  named-pipe dogfood pass, nothing in this session hit a genuine
  kernel-level blocking wait that needed Ctrl-C/Ctrl-E at all (every
  observed "stuck" reader was actually a live, fast-retrying userland
  loop, confirmed by its own advancing retry counter and timestamps,
  not a suspended process) -- so the restart above was to recover a
  *flooded, hard-to-read terminal*, not to break a hung syscall. Worth
  noting as a genuinely different failure class from the SAFETY WARNING
  this task was briefed with, even though the recovery command was the
  same.
- Compile workflow (`load math`, `LOAD`/`PACK`/`SAVE`, the `Rewrite?:`
  confirmation prompt on re-PACK/re-SAVE) matched `pack-and-runb.md`
  exactly, first try, across two full recompiles (once for the initial
  18-record/unthrottled version, once for the 40-record/throttled
  version after the restart).

## Overall

The skill's qualitative model of *what gets locked and when* held up
under everything this pass could check directly, but the doc has a real
gap on the single question this task cared about most -- what a reader
actually *experiences* at the moment it catches up to a live writer --
and live behavior there turned out to be worse than either outcome the
task was designed to distinguish between. This reads as a genuine,
cleanly-reproducible `os9exec` RBF bug (concurrently-opened reader paths
never gaining visibility into a growing file, even after the writer
closes) rather than a faithful reimplementation of the original
pipe-like design intent. Per the task's scope, no fix was attempted;
this report and the three archived `.bas` files are the diagnostic
handoff.
