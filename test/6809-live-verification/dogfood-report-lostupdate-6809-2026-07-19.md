# Dogfood report: RBF automatic record-locking, lost-update race, NitrOS-9 (6809) (2026-07-19)

Direct 6809 port of the 68k pass
(`test/68k-live-verification/dogfood-report-lostupdate-2026-07-18.md`),
answering the question that pass's own report left explicitly open: does
`os9-systems-dev/file-managers.md`'s Record Locking "case 1" design intent
(a database-style read-modify-write cycle is race-safe under concurrent
access with **zero** explicit `SS_Lock` calls, because `Read` in update
mode locks the record just read and the following `Write` releases it)
hold on **NitrOS-9's** RBF (community-written clone of OS-9 Level 2, not
licensed Microware source), or only on `os9exec`'s independent 68k
reimplementation? The 68k test hit exactly 2×N=600 twice — this was
previously untested on NitrOS-9/6809.

Test shape (identical to the 68k original, only the `TYPE` field width
differs — 6809 `INTEGER` is 2 bytes, not 4): `dogfood-lostupdate-init-6809.bas`
creates a 10-byte record (`count` + 4 `INTEGER` filler fields, pushing
`count`'s byte range away from the file's true EOF so writes land
mid-file, not on the separate EOF-lock path); two instances of
`dogfood-lostupdate-incrementer-6809.bas`, each doing an **unprotected**
`SEEK 0 / GET / +1 / SEEK 0 / PUT` loop for N=300 iterations with the path
held open throughout and no `SS_Lock` anywhere, race concurrently against
the same file; `dogfood-lostupdate-verify-6809.bas` checks the final
count. Target: real RBF on the EOU test disk (`/dd/CLAUDE/counter.dat`,
not a host-native mount — this platform's `/dd` is always a real IDE-backed
RBF image, so the host-native-vs-real distinction that mattered for the
68k pass doesn't apply here at all).

## Result: does NOT reproduce case 1 — real, reproducible lost updates

**Two independent races, both short of the expected 600, by different
amounts:**

```
Race 1: lostinc & lostinc &
  lostinc: done, 300 iterations, last local count seen=340
  lostinc: done, 300 iterations, last local count seen=457
  lostver: final count=457                    (expected 600, lost 143)

Race 2 (counter reset to 0 in between):
  lostinc: done, 300 iterations, last local count seen=564
  lostinc: done, 300 iterations, last local count seen=584
  lostver: final count=584                    (expected 600, lost 16)
```

This directly contradicts the 68k result on the identical test design.
Two things rule out the obvious alternative explanations:

- **Not a serialization/no-real-overlap artifact.** The 68k pass's own
  report flagged a weakness: its two racers' "last local count seen"
  values were a clean 300 then 600, meaning it had no *direct* evidence
  of genuine mid-race interleaving (both racers could have run
  essentially back-to-back). This 6809 run doesn't have that weakness —
  340/457 and 564/584 are neither racer's own iteration count, proving
  real interleaved access to the shared record: each racer's last
  observed value reflects wherever the *other* racer's writes had also
  landed by that point.
- **Not a silently-swallowed I/O error in the test program.** Neither
  racer sets `ON ERROR GOTO`. Per `basic09/basic09-language.md`'s Error
  Handling section, a non-fatal error with no trap armed drops the
  process into interactive Debug Mode — which would have visibly halted
  the run, not let it print a clean "done, 300 iterations" and exit. Both
  racers completed cleanly in both races, with no crash, no Debug Mode
  interruption, no visible error text. **This means the lock contention
  is not erroring — it is genuinely not preventing the interleaved
  read-modify-write.** Whatever's happening, it isn't "GET fails during
  contention and the untested program silently carries on with stale
  data" — that theory is ruled out by the absence of any trap/crash.

The variability between the two races (143 lost vs. 16 lost) is itself
informative: consistent with a genuine, timing-dependent race condition
(more actual interleaving on one run than the other), not a deterministic
bug that would lose the same count every time.

## What this means for the design-intent claim

**Case 1 of `file-managers.md`'s Record Locking section is confirmed
`Live` on `os9exec`/68k, and now confirmed to NOT hold the same way on
NitrOS-9/6809, in this exact test.** Scope this precisely: NitrOS-9 is an
independent community-written clone of OS-9 Level 2, not licensed
Microware source, exactly the same relationship `os9exec` has to genuine
68k OS-9 — this is a finding about two different reimplementations
diverging from the same firsthand design intent in different ways, not
"6809 gets locking wrong" or "68k gets it right." (The sibling EOF-lock
pass found the opposite asymmetry: `os9exec` has a real bug there and
NitrOS-9 doesn't — so neither reimplementation is "the correct one" in
general.) The mechanism producing this divergence is not yet determined —
candidates not distinguished by this pass: NitrOS-9's RBF automatic
per-record lock genuinely isn't being acquired or enforced for this
access pattern; some layer of BASIC09-on-6809's own `GET`/`PUT`
implementation doesn't route through the lock-acquiring path the way the
design intent (and `os9exec`'s reimplementation) expects; or a
6809-specific buffering/caching layer serves a stale in-memory copy on
`GET` without re-consulting the file. Settling which would need either a
raw-syscall (`I$Read`/`I$Write`, bypassing BASIC09's own I/O layer
entirely — the `syscall-*.a` test infrastructure from the same session
this report was written in could be extended for exactly this) version of
the same race, or checking whatever's actually shipped in this NitrOS-9
build's RBF file manager source.

## Harness notes specific to this pass

- **`LOAD`'s literal-filename-match bit twice.** The `.bas` extension on
  the host-side files must NOT be carried onto the OS-9 disk — `LOAD
  lostinit` fails `Error #216` against a file literally named
  `lostinit.bas`; it needs to be named `lostinit` with no extension.
  Already documented in `using-os9exec-repl.md`'s BASIC09 section
  ("`LOAD` does a literal name match — no extension inference") but easy
  to trip over anyway when injecting via ToolShed with the host filename
  used as the on-disk target name.
- **A fresh `basic09` invocation has an empty program** — `RUN <name>`
  without a preceding `LOAD <name>` in *that* session fails `Error #043
  -- Unknown Procedure`, even if the same name was `LOAD`ed and `RUN` in
  an earlier `basic09` session that has since `BYE`d out. Obvious in
  hindsight, cost one wasted round trip here.
- **The buried-shell-prompt gotcha (already documented in
  `using-nitros9-repl.md`) reproduced exactly as described**: immediately
  after `lostinc & lostinc &` finished and printed both "done" lines, the
  next `send 'lostver'` timed out even though the shell was genuinely
  idle — a plain `key Enter` recovered it before the retry succeeded.
