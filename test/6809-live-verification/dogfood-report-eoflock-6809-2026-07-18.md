# Dogfood report: RBF EOF-lock design intent, real NitrOS-9 (6809), 2026-07-18

Task: repeat the OS-9/68000 `os9exec` EOF-lock dogfood pass
(`test/68k-live-verification/dogfood-report-eoflock-2026-07-18.md`)
against **real NitrOS-9 running on 6809** (XRoar + DriveWire, via
`tools/nitros9repl.sh`), not `os9exec`. This project's owner personally
designed the original Microware EOF-lock mechanism; the 68k pass found a
severe, reproducible `os9exec` bug (a reader path opened concurrently
with an active writer never sees any of the writer's data, for the
reader's entire remaining lifetime, even long after the writer closes).
The question here: does the real, original file manager honor the
pipe-like design intent that `os9exec`'s reimplementation apparently
doesn't, or is some version of this limitation inherent to the
mechanism itself?

Two BASIC09 programs (READ-only reader only, per the task's scope --
the 68k pass found UPDATE mode made no observable difference, so it
wasn't repeated here), adapted from the 68k originals:
`dogfood-eoflock-writer-6809.bas` (A, appends 13 timestamped records,
one per real second, to `/DD/EOFTEST/eoflock.dat`) and
`dogfood-eoflock-reader-6809.bas` (B, opens the same file `:READ` and
loops reading, throttled-polling on `E$EOF`/211 via `ON ERROR GOTO`).
Each file's header carries its own literal verified output; this report
is the narrative/analysis layer, same convention as the 68k trio.

## What was actually observed

**Two independent clean runs, identical result each time:**

- Writer A launched, reader B launched in the same real second (combined
  `eofwr & eofrdro &` shell command line, one `key` call -- see Harness
  friction below for why this matters). A appended 13 records, one per
  real second, `err=0` every time, and closed cleanly -- exactly as
  boring and flawless as the 68k pass's writer.
- Reader B's `OPEN` succeeded immediately (`err=0`), in the same real
  second as A's first `WRITE`.
- Reader B's **very first `READ` call did not return for roughly 12
  real seconds** -- and when it finally did, **`retries` was 0**: the
  `ON ERROR GOTO 800` handler (which increments `retries` and prints an
  `EOF retry #N` line on `E$EOF`) never fired, not even once, during
  that entire wait. The read simply didn't return, then did.
- The moment it returned, **all 13 records came back immediately, in
  order, correct content, zero further retries**, timestamped to the
  same real second as the writer's 13th `WRITE` and its `CLOSE`.
- Reproduced identically in a second full run (fresh file, same
  combined-launch method): reader opened same-second as writer's first
  write, first successful read landed in the same real second as the
  writer's close, again `retries=0` throughout.
- A supplementary run confirmed the ordinary, uncontested baseline: a
  reader opened well *after* the writer had already closed reads all 13
  records instantly, `retries=0`, no surprises -- ruling out any data
  corruption or file-level problem, same as the 68k pass found for its
  own after-close case.

## Does this match the design intent, the 68k bug, or neither?

**Neither, and it's a third distinct outcome again** -- but a much
healthier one than `os9exec`'s:

1. **It is not `os9exec`'s bug.** The 68k reimplementation left a
   concurrently-opened reader blind to the writer's data *forever*,
   including long after the writer closed. Real 6809 RBF eventually
   delivers every byte the writer wrote to the already-open reader path,
   correctly, every time this was tried.
2. **It is not the immediate-return/poll-yourself mechanism the 68k
   pass found either.** `retries=0` across every clean run means the
   reader's `READ` call itself did not return `E$EOF` and get retried
   by the program's own loop -- it appears to have genuinely blocked at
   the kernel level. This directly answers a gap the 68k pass flagged in
   `file-managers.md`'s Record Locking section (the doc said nothing
   about whether a reader catching up to a live writer blocks or
   returns immediately) -- on real 6809 RBF, the answer is **it blocks**.
3. **It is also not quite the idealized pipe-like design intent as
   described.** The owner's design intent is that a reader should freely
   read everything already flushed and only stall at the live edge. Here,
   record #1 was fully written and closed-out (by A's very next write
   starting) a full ~11-12 seconds before B's read of it finally
   returned -- record #1 was never "at the live edge" from B's
   perspective once A had moved on to seq=2, 3, 4, etc. Under the
   stated design intent there's no reason a read of already-committed
   data should need to wait at all. What was actually observed reads
   much closer to **"B is blocked for the entirety of A's open path,
   and everything unblocks at once when A closes"** -- i.e. closer to
   the "locked out until the writer exits" fallback candidate that the
   original 68k task set out to distinguish from the pipe-like ideal,
   except here it's a real blocking wait (not polling) and it *does*
   eventually deliver every record correctly, rather than never
   delivering anything.

**Why prefer "genuine kernel block" over "the reader was just
CPU-starved by the writer's busy-wait loop" as the explanation for the
~12-second gap?** Both programs' polling loops (`IF t=lastt THEN 100`
for the writer, the throttled `FOR k=1 TO 1000: NEXT k` for the reader)
are busy-loops that could in principle starve one another under a
naive scheduler. But the reader's `OPEN` and its `PRINT` announcing it
executed immediately, in the same second as the writer's first write --
so the reader process clearly *was* being scheduled at the start, not
starved from the outset. If it had merely been getting occasional,
CPU-starved timeslices for the next 12 seconds, the tiny 1000-iteration
retry throttle should have let at least one or two `E$EOF`/retry cycles
complete somewhere in that window; instead there were exactly zero,
every time, in both clean runs. That pattern -- runs immediately, then
totally silent with no error activity for ~12 seconds, then completes
instantly and correctly -- is much more consistent with the `READ` call
itself being suspended in a real kernel wait than with a starved
polling loop that just got unlucky twice in a row. This isn't chased
further at the kernel-source level (matching the 68k report's own
decision not to root-cause its bug further); it's the most likely
reading of the evidence in hand, not a certainty.

## Was `file-managers.md`'s Record Locking section wrong, missing, or confusing?

The section (already updated once after the 68k pass, see its existing
`os9exec`-behavior paragraph) said nothing had been confirmed about
whether a real, non-`os9exec` RBF actually blocks or polls at this
point -- that gap is now filled with a real data point, added below with
a `Live` tag. The doc's "mechanics" list (a write always releases any
lock the path holds, an EOF lock is acquired only by a write landing at
true EOF) doesn't by itself explain why a read of *already-committed,
non-edge* data would block until the writer's *close* rather than until
the writer's *next write* (which should release the prior EOF lock much
sooner, per that same mechanics list, if the release happens promptly at
the start of the next write rather than being held until something
else). This live result suggests either the EOF lock is held more
continuously by an actively-writing path than the current doc phrasing
implies, or there's a separate, undocumented mechanism gating a
different-path's stalled read until path closure specifically. Flagged
for whoever looks at RBF's actual lock-table code next; not resolved
further here.

## Harness friction (real, worth keeping)

- **A real bug found and fixed in `tools/nitros9repl.sh` itself,
  independent of the eoflock question**: its `tmux_escape()` helper
  replaced every `;` in a `key`/`send` argument with `\;`, apparently on
  the assumption that tmux's own command-line semicolon-as-separator
  syntax needed escaping. Live-tested this directly (a throwaway tmux
  session, `tmux send-keys -l -- "echo a;b"`, confirmed tmux delivers the
  full literal string with no re-splitting when passed as one
  already-quoted bash argv element via `-- "$string"`) -- the escaping
  was unnecessary and actively harmful: it typed a literal backslash
  into the guest before every semicolon, corrupting any BASIC09 line
  using `;` as a `PRINT` separator (i.e. nearly every diagnostic print
  in these two programs). First attempt at typing
  `dogfood-eoflock-writer-6809.bas` this way landed on disk with `\;`
  everywhere a real `;` was intended (confirmed via the OS-9 shell's own
  `list` on the raw text file, not just BASIC09's internal display).
  Fixed by making `tmux_escape` a no-op (with a comment explaining why);
  re-verified clean with a small `tee`+`PRINT #2, "a="; 1; " b="; 2`
  probe before retyping the real files. This is a durable fix, not a
  workaround -- every future `key`/`send` call with a semicolon in it
  benefits.
- **Combined single-launch-line trick (`progA & progB &` in one `key`
  call) worked identically well here as it did on the 68k `os9repl.sh`**
  -- both processes started within the same real second, no race, every
  time it was used.
- **The OS-9 shell's own prompt gets "buried" under backgrounded jobs'
  streamed output and `send`/`wait_prompt` will then time out on the
  very next command, even though the shell is genuinely idle and ready**
  -- not previously documented for this harness. Once both backgrounded
  jobs finish printing, the last visible pane line is whatever their
  last `PRINT` was, not a fresh `{N1|NN}path:` prompt row (the shell
  already printed its prompt once, immediately after backgrounding,
  before either job's output existed to bury it). A plain `key Enter`
  reliably brings a fresh, matchable prompt back. Worth adding to
  `using-nitros9-repl.md` as a named gotcha -- this cost several
  `[TIMEOUT waiting for prompt]` cycles in this session before the
  pattern was recognized.
- **A leftover, unrelated `nitros9repl` tmux/XRoar session from a prior
  task (6809 GFX2 windowing work, mid-`Error #184` at a `B:` prompt) was
  already running** when this session started; it was stopped via a
  plain `restart` per this task's explicit authorization to do so if
  needed for recovery -- disk image state persisted as documented, no
  GFX2 work appeared to be lost (its own plan/cards directory was a day
  stale, not concurrently in use).
- `BYE` (capitalized) is the correct command to leave interactive
  `basic09` back to the OS-9 shell -- lowercase `quit`/`bye` both
  produce a silent `What?` with no visible effect via `send` (since
  `send`'s prompt-gate doesn't recognize `B:` either way, so a failed
  attempt looks identical to a slow one until you `peek`). Worth a
  one-line mention in `using-nitros9-repl.md` alongside the existing
  `E:`/`DB:`/`Topic:` sub-prompt list, since exiting `basic09` is a
  near-universal step in any BASIC09 dogfood session.
- No `math`-trap-handler banner or residency issue was hit this session
  (unlike the 68k pass's `load math` gotcha) -- 6809 BASIC09 appears to
  handle numeric declarations/arithmetic without needing an equivalent
  preload step; `/dd/cmds` on this disk image doesn't even contain a
  `math` module. Not chased further; flagged in case it matters for a
  future 6809 numeric-heavy dogfood pass.

## Overall

Real, original NitrOS-9 RBF on 6809 does **not** reproduce `os9exec`'s
bug -- a concurrently-opened reader path eventually sees every byte the
writer produced, correctly, every time. That's the important
confirmation for `os9exec`: this is very likely an `os9exec`-specific
regression in its RBF reimplementation, not an inherent limitation of
the EOF-lock mechanism itself. But the real mechanism's actual behavior
is also not a clean match for the idealized pipe-like design intent as
literally described -- it looks like a real, kernel-level blocking wait
that releases only once the writer's path closes, rather than promptly
unblocking after each individual write the way a true pipe would. Per
this task's scope, no further root-causing was attempted; this report
and the two archived `.bas` files (plus the `tools/nitros9repl.sh` fix)
are the diagnostic handoff.
