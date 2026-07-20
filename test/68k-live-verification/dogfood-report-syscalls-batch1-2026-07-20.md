# Dogfood report: 68k syscall live-verification batch 1 (2026-07-20)

Starts a systematic live audit of `68k/syscall-reference.md`, which
until now had zero individually-live-tested calls despite the header
claiming `Manual` confidence throughout (several calls had been
*incidentally* exercised by earlier BASIC09/C/assembly dogfood sessions
without ever being credited `Live` in the reference file — those were
backfilled first, see below, before any new testing).

**Methodology note, the main finding of this whole session**:
`os9exec` is this project's own emulator, with its C source right here
in the repo (`Source/OS9exec_core/`) — unlike the 6809 side (NitrOS-9,
a third-party clone), there is no ambiguity about "what should the real
implementation do," only "does the live behavior match the source, and
does the source match the docs." Checking `fcalls.c`/`funcdispatch.c`
directly, *before* guessing a register convention in test code, resolved
two calls immediately and definitively that would otherwise have stayed
"inconclusive" the way several 6809 findings did.

## Free backfill (no new testing) — 7 calls now `Live`

`I$Open`, `I$Read`, `I$Write`, `I$Close`, `F$Exit` (all from
`dogfood-asm-line-counter.a`, 2026-07-18 — real files, byte-exact
line/char counts against independently-verified host data), `I$SetStt`
(SS_Lock/SS_Ticks, from `dogfood-sslock.a`/`dogfood-ssticks.a` — found
and fixed a real `os9exec` bug, `SS_Lock` claiming success while doing
nothing), `F$STrap` (from `dogfood-strap-unterminated.a` plus the
BASIC09 REAL÷0 fix work — found and fixed a real out-of-bounds read).

## New live testing — `batch1-01.a`

### F$ID — fully resolved

Register convention had zero detail in the skill (`Manual`-only).
Checked `OS9_F_ID` directly: `d0.w`=PID, `d1.l`=group.user (packed,
group high word), `d2.w`=priority. Live output (`d0=6, d1=263, d2=128`)
matches exactly — `263` decodes to group=0/user=263, `128` is a
plausible default priority. Promoted out of the `Manual`-only list into
its own confirmed row.

### F$Time — fully resolved, first guess was wrong

Also zero register detail in the skill. First attempt assumed a
6809-style 6-byte-buffer-pointer convention (by analogy) — live-tested,
got a real "PASS" (carry clear) but an implausible year value (0),
proving the guess wrong rather than confirming it. Checked
`OS9_F_Time` directly: `d0.w`=mode (input), `d0.l`=packed time,
`d1.l`=packed date, `d2.w`=day-of-week, `d3.l`=ticks (all output,
registers only, no buffer at all). Rewrote the test to match; live
result now a real nonzero packed time/date, carry clear. New `## Time`
section added to the reference file. Packed-field bit layout not
decoded this pass — only the register-slot convention is confirmed.

### F$CmpNam — fully resolved, two real findings plus one self-inflicted bug

Already had register detail documented (`Manual`), so this was meant to
be a quick confirmation — instead surfaced the most involved
investigation of this batch:

1. First attempt (identical-string self-comparison, expecting a match)
   **failed** — reported mismatch.
2. Ported the exact sign-bit-terminator theory from a matching,
   never-fully-resolved 6809 `F$CmpNam` finding (batch 3, same session
   history) — rebuilt the test strings with the sign bit baked into the
   last character instead of a separate trailing byte. **Still failed**,
   refuting that theory for 68k (and, by extension, casting doubt on it
   for 6809 too — it was never actually confirmed there either).
3. Ruled out a same-memory-address edge case (compared two separate
   buffers with identical content). **Still failed.**
4. Checked `OS9_F_CmpNam` directly: the implementation scans the
   **target** string for a literal `0x00` NUL byte — no sign-bit
   handling anywhere in the function. The **pattern** is purely
   length-bounded by `d1.w`, no terminator needed. Rebuilt the test
   strings with real NUL termination, matching this exactly. **Still
   failed** — same result, which was genuinely puzzling since the
   corrected construction traced through the algorithm by hand as a
   guaranteed match.
5. Confirmed via `dump` on the linked module that the string bytes were
   byte-for-byte correct in memory (`74 65 73 74 00` = `"test\0"` in
   both places being compared).
6. At the time, switching `d1`'s load from `move.w #4,d1` to
   `moveq #4,d1` (clearing the whole register first) appeared to fix
   it immediately, and this report originally concluded that was the
   real cause — a register-discipline bug, the 68k analog of the 6809
   suite's `,U`-clobber pattern.

**CORRECTED, same day, after a dedicated follow-up isolation test
(`batch4-cmpnam-isolate.a`): that diagnosis was wrong.** `OS9_F_CmpNam`
reads the length via `loword(rp->d[1])`, a direct 16-bit memory read
genuinely insensitive to `d1`'s upper word — confirmed by deliberately
dirtying `d1`'s upper word (synthetically, and via a real preceding
`F$Time` call) immediately before a plain `move.w #4,d1`, and
`F$CmpNam` matched correctly regardless, using the exact same
NUL-terminated strings from step 4. **The NUL-termination fix in step 4
was the real, sole fix all along.** The apparent "still failed" result
at step 4 was almost certainly a false signal from the `r68 -O=`
stale-rebuild bug (see Toolchain notes below) — this investigation is
where that bug was first discovered, and the timing lines up: reliable
`del`-before-rebuild discipline only started around the same point as
the `moveq` change, not because of it. **No general 68k
register-clearing rule is established by `F$CmpNam`.** Left in this
report as a methodology lesson: when a fix seems to work right after
*also* changing something toolchain-related, suspect the toolchain
before trusting the code explanation — verify with an isolated
follow-up test before writing the conclusion into a skill file, which
is exactly what this correction did after the fact.

## Real toolchain gotchas found (not syscall findings)

- **`(pc)`-relative addressing is required for every local-label
  reference**, exactly like `,pcr` on the 6809 side — forgetting it
  produces "absolute addressing" warnings (not hard errors, easy to
  miss) and, per the established 6809-side lesson, risks non-relocatable
  behavior. Applied consistently after the first assembly pass flagged
  it.
- **`ds.b` is not a valid directive on this `r68`** — `*** error - bad
  mnemonic ***`. Used `dc.b 0,0,...` (explicit zero bytes) instead for
  scratch buffer reservation.
- **`r68 -O=<name>.r` does not reliably overwrite an existing `.r`
  file of the same name** — re-assembling without first `del`eting the
  old object file left `l68` unable to link it (`error - file 'X.r' is
  not a relocatable module`) despite `r68` itself reporting `0 errors`
  and a fresh, correctly-sized/timestamped file. Reproduced twice.
  Always `del` the old `.r`/module before re-assembling/re-linking under
  the same name, matching the already-known "delete before recreate"
  discipline that applies to ToolShed on the 6809 side too, just a
  different toolchain hitting the same class of bug.
- **`l68 -o=<bare-name>` is unreliable about where it writes** —
  confirmed the earlier filemgr session's finding: whether it lands in
  the current directory depends on account-specific `chx` state in a way
  that isn't obvious. Used explicit full paths for every link this
  session rather than relying on it.

## Files

- `test/68k-live-verification/childprg68k.a` — trivial F$Exit(77) helper, for future F$Fork/F$Load/F$Link tests
- `test/68k-live-verification/batch1-01.a` — F$ID, F$Time, F$CmpNam
