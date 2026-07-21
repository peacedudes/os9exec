# NitrOS-9 6809 record-lock lost-update — handoff (2026-07-21)

The mode-gating work is DONE and separate (commits `48928ab`..`b45a9e1`,
`docs/nitros9-rbf-lockmode.patch`, PR awaiting the owner). This thread is the
defect that patch does NOT fix.

## The defect

Two update-mode racers, each 200 read-modify-write cycles on one shared record
(`SEEK 0 / GET / +1 / SEEK 0 / PUT`), self-verifying completion: **332 of 400,
68 updates lost**, stable, both racers confirmed finished. On the three-gate
patched module; stock loses the same way (prior sessions). Meanwhile the lock
demonstrably blocks when held: `rlwait` contended with `rlhold2` sleeps 62s and
reads the post-write value. So updates are lost inside the GET→PUT window, not
because locking is absent.

## First hypothesis to test (cheap)

`rlrace3` does a `SEEK` between the GET and the PUT. Does NitrOS-9's `I$Seek`
release the record lock? By the owner's design a read holds the record lock
until the NEXT WRITE on that path — a SEEK should not release it. Check
`level2/modules/rbf.asm` Seek entry for lock/`PE.Lock` manipulation, then try a
racer variant with no SEEK (records are at position 0; GET leaves position 10 —
use `PUT` after a rewinding GET pattern that avoids SEEK, or a 1-record file
where position wraps naturally). If SEEK releases, the fix conversation is with
the owner first — design authority, ask rather than infer. Second suspect: the
sleep/retry conflict walk (`L0B0C`) granting a lock to a waiter while the
holder is between release and reacquisition ("reacquisition-timing gap", see
memory `rbf-lostupdate-6809-diverges-from-68k`).

## Environment (cold start)

Follow `docs/nitros9-rbf-lock-HANDOFF.md` "Reproducing from a cold start"
verbatim (private clone of the image, private tmux/ports, install
`docs/nitros9-rbf-reference/rbf.patched-testdisk.mn` — keep the mode gates in
while hunting this). Fixtures needed: `rl-init.bas`, `rl-race3.bas`,
`rl-check.bas` (+ `rl-hold2.bas`/`rl-wait.bas` for the blocking control), all
in `test/6809-live-verification/`, uploaded with `tools/nitros9-upload-b09.sh`.

Run shape: `del rl.dat; rlinit; rlrace3 >>>/nil&; rlrace3` (foreground returns
in ~4s), wait ~10s, `rlcheck` twice (same number twice = both racers done).

## Hard-won gotchas beyond the old handoff's list

- **`chx` can silently not take**; a later `pack` then fails `#214` writing to
  /dd/CMDS. Verify chx before packing. Copy `basic09`+`runb` into the workdir
  (chx away from CMDS loses every utility).
- **Channel wedge**, two flavors seen: guest sends SS.Relea and goes deaf, or
  output goes stale/bundled. Either way: `restart`, don't diagnose. Disk state
  survives; re-login + chd + chx after.
- **`>>>path` redirects stdout+stderr; `<>>>path` all three** (owner-taught,
  live-verified). A `>+` redirect is append/WRITE-only, NOT update — it takes
  no locks post-patch.
- **Cheap real sleep exists**: `RUN syscall(10, regs)` = F$Sleep, `regs.x` in
  60ths (see `rl-stamp.bas`). Prefer it over `FOR j` busy-loops for pacing.
- **Untrapped BASIC09 I/O deaths lie**: runb's abort issues SS.Size at the
  current position — wrong exit code on READ paths, file resize on UPDATE
  paths (memory `nitros9-runb-abort-sssize`). Fixtures that can die at EOF
  need `ON ERROR` if their exit code or the file size matters.
- The os9exec (68k) EOF-lock bug in `ROADMAP.md` is a DIFFERENT implementation
  and a different defect — don't conflate the two while reading around.

## Posture (owner's standing rules)

Make every check fail once before trusting it. A/B/A any before/after. A red
result does not accuse RBF until the harness is exonerated. The owner is the
design authority — ask about intended behavior, don't infer it.
