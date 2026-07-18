# Dogfood report: hand-written 6809 assembly, raw I$ syscalls (2026-07-18)

Task: write a 6809 OS-9 assembly program from scratch (not BASIC09) that
opens a file named by a command-line parameter, counts lines and
characters (excluding CR terminators) using raw `I$` syscalls, prints a
summary, and exits — using only the `os9-dev` skill plus live testing via
`tools/nitros9repl.sh`, no prior conversation context about this specific
program. Working source is
`test/6809-live-verification/dogfood-asm-line-counter.a`, with its own
header comment giving the literal verified output.

This is a companion to `dogfood-report-2026-07-18.md` (a same-day BASIC09
dogfood pass); this one specifically targets the newly-expanded 6809
Assembler Directives / RMA-vs-`asm` / DEFS-files content in
`6809/assembly-and-tools.md`.

## Did the skill get me there first-try, or did I have to guess/experiment?

Not first-try. Three real assemble/run cycles for the final program (plus
two more for a throwaway parameter-probe program written first), with two
genuine logic bugs found only by live debugging, not by re-reading source.

**What worked immediately, no experimentation needed:**
- Which assembler to use. The skill is unambiguous ("asm fully works
  through this REPL, rma doesn't") and gives the exact working invocation
  syntax (`asm filename [opts] [#memsize] [>listing]`, `O=name` for
  output, leading `-` for silent overwrite). I never touched `rma` and
  never needed to — zero wasted cycles here, which is exactly the value
  a skill should add over blind trial.
- The `O=` writes to CMDS, not the current directory" gotcha. The skill
  states this exact trap ("every run looks like a silent failure — check
  `CMDS/name`") and I would have hit it blind otherwise; instead I went
  straight to `fsize CMDS/pdump` and `ident CMDS/lncnt`.
- `SWI2` + `FCB <code>` dispatch convention and the specific codes I used
  (`I$Open`=$84, `I$ReadLn`=$8B, `I$Close`=$8F, `I$WritLn`=$8C,
  `F$Exit`=$06) — all correct on the first try, sourced from
  `6809/syscalls-and-module-format.md`'s table.

**What required real trial-and-error, with literal errors hit:**

1. **Module header size must include EMOD's 3-byte CRC trailer** — not
   stated as an explicit rule anywhere in `assembly-and-tools.md`'s `MOD`
   row. I wrote `mod eom,nm,$11,$81,start,$0080` with `eom equ *` placed
   *before* `emod` (mirroring `sigtest.a`'s structure but missing its
   `modend+3` — I copied the label pattern, not the `+3`). Assembled clean
   (`00000 error(s)`), but `ident` showed the truth:
   ```
   Header for:
   Module CRC:  $898D03 (Bad)
   Edition:     $CD      #205
   Module header is incorrect!
   ```
   and the shell refused to run it at all (`Error #235 - Bad Name`). I
   only found the fix by diffing against `ident`'s output for the
   already-known-good `sigtest.a`, which uses `modend+3` as the MOD
   size operand. The skill's `MOD` row documents *what* the directive
   does but never flags that the assembler-computed `eom`/`modend` label,
   if placed before `EMOD`, is 3 bytes short of the real module size —
   an invisible trap for anyone not already comparing against a working
   example. Worth adding explicitly.

2. **Short branches out of range** — once the real read-loop/print logic
   pushed my `noparm` error handler more than 127 bytes from its three
   callers, `asm` correctly caught it:
   ```
   ***** Error: out of range
   00017 E 0015 27FE                beq   noparm
   ```
   (×3). Fixed by switching those three to `LBEQ`. The skill's
   directives table documents `IFxx`/branches and separately warns about
   phasing errors and `LBxx`-vs-short-branch listing hints, but doesn't
   connect the dots into an explicit "a short conditional branch to a
   label more than ~127 bytes away will error at assemble time, use the
   `L`-prefixed form" rule. Not wrong, just a gap a first-time reader
   would trip over exactly once (which I did).

3. **Command-line parameter register convention — genuinely unresolved,
   even after live testing.** The skill honestly flags this as uncertain
   (`"U/X = bottom (direct page/data area boundaries depending on
   source)"`), so this isn't a case of the skill asserting something
   wrong — it's a real gap I had to fill live, and what I found was
   *inconsistent between two different ways of invoking the same
   program*:
   - Via a real shell fork (`pdump2 ABCDEFGHIJKLMNOP` typed at the OS-9
     prompt), a probe program dumped 16 bytes starting exactly at entry
     `X` and got `4142434445464748494A4B4C4D4E4F50` — i.e. `X` pointed
     directly at the argument text, no leading space, with entry `D`
     (param length) = 17 = 1(implied leading byte not included in the
     dump)+16.
   - Via the OS-9 debugger's `E LnCnt lccount1` (load-for-execution),
     a raw memory dump at entry `X` showed
     `20 6C 63 63 6F 75 6E 74 31 0D` — a literal leading **space**
     (`$20`) at `X` itself, then `lccount1`, then a trailing CR, with
     entry `D`=10 = 1(space)+8(text)+1(CR).
   I never fully reconciled why `E` and a real shell fork appear to set
   up the parameter area differently (one byte offset apart, plus `E`
   apparently appends a trailing CR the shell fork didn't). Rather than
   trust either convention blindly, the final program peeks the first
   byte and conditionally skips it only if it's actually a space — this
   works correctly under both invocation paths and is documented inline.
   **This is a real open question for the skill to eventually resolve
   with more targeted live testing** (e.g. a probe run only via real
   shell invocation with several different argument lengths) — I didn't
   have budget to chase it further once I had a working defensive fix.

4. **`I$ReadLn`'s returned length includes the CR terminator** — not
   stated either way in the skill's `I$ReadLn` row (`"Reads to CR, with
   line editing"`, no explicit statement about what the returned `Y`
   excludes). My first working-logic version summed `Y` directly into
   the character count and got `Chars: 00078` against an independently
   `fsize`-verified 75; the discrepancy was off by exactly one per line
   (3 lines, +3), which was the tell. BASIC09's `READ`/`LEN()` (verified
   in the same-day BASIC09 dogfood pass, `dogfood-file-line-counter.bas`)
   *does* exclude the terminator, so this is a real, useful divergence
   between the raw syscall and BASIC09's higher-level wrapper around it —
   worth adding to `6809/syscalls-and-module-format.md`'s `I$ReadLn` row.

5. **A genuine logic bug with no assembler diagnostic at all**, found
   only by live register/memory inspection: my decimal-printing
   subroutine reloaded the 16-bit value being divided via
   `LDD VALUE,u` on every inner-loop pass, without accounting for the
   fact that `LDD` overwrites *both* halves of `D` — including `A`,
   which the same loop was using (via `INCA`) as its digit accumulator.
   Every iteration silently wiped the count back to whatever the high
   byte of the (partially subtracted, wrapping) value happened to be.
   Symptom was not a crash — the program ran to completion and printed
   plausible-looking garbage: `Lines:,///` / `Chars: <TAB>,///` instead
   of digits. Confirmed live via the debugger: breaking at the point
   right after the buggy subtraction (`prddn`) showed
   `A=$D8` (216 decimal) for a value that should have produced digit
   `0` — impossible to reach via 216 *real* successful subtractions of
   10000 from a 16-bit register starting at 3, which is what led me to
   suspect register-clobbering rather than an arithmetic error. Fixed
   by moving the digit accumulator to its own scratch byte separate from
   the register used for the division. This is a 6809-CPU-semantics bug,
   not an OS-9 convention bug — no skill document could have prevented
   it; only the debugger caught it.

## Was anything actually wrong (not just missing) in the skill?

Nothing I found was factually *wrong* per se — the MOD-size-needs-`+3`
and short-branch-range gaps above are both *omissions*, not incorrect
claims, and the parameter-register uncertainty was already honestly
flagged rather than asserted confidently. That's a meaningfully different
failure mode from the skill asserting something false, and worth noting
as a point in the skill's favor even though it cost me real cycles.

## Was anything surprisingly helpful?

- The `rma`-hangs-use-`asm` guidance saved what would likely have been
  an 8+ minute dead end (matching the skill's own documented hang time)
  — I never attempted `rma` at all.
- The debugger's `E <modname> [params]` — "load a program for execution
  ... shows initial register dump" — being confirmed `Live` in the skill
  turned out to be the single most useful tool in this whole session: it
  let me see real entry-time `X`/`Y`/`U`/`D` values and, combined with
  `B <addr>` breakpoints and `M <range>` memory dumps, let me catch the
  `LDD`-clobbering bug by direct observation (`A=$D8` at a point where it
  had to be `$00`) rather than by more paper-tracing, which had already
  failed to find it across several careful manual passes.
- The `L <name>` — "only finds modules already in the live module
  directory" — caution was directly hit and confirmed exactly as
  described (`Error #221 - Module Not Found` right after `debug`), and
  saved time recognizing it immediately rather than assuming the REPL
  channel was broken. One small extension worth adding: **this also
  applies to `E`**, not just `L` — `E LnCnt lccount1` gave the identical
  `Error #221` until I explicitly `load`ed the module first. Also, a
  module resident via one `L`/`E`/shell-run isn't fully released by a
  single `unlink` if it was linked multiple times — I needed **four**
  `unlink LnCnt` calls before `mdir` stopped showing it, presumably one
  per `L`/`E`/`E`/shell-run linking event across the session; not
  documented anywhere and worth a line in the debugger section.

## Did I hit the REPL sub-prompt timeout issue?

Not directly, because the skill's warning was specific and I followed it
preemptively: I never used `send` on `debug`, `tee`, or any sub-prompt —
I used `key` for every debugger/tee interaction from the start, exactly
as `6809/using-nitros9-repl.md` instructs. Zero `[TIMEOUT]`s from that
particular failure mode all session.

I did hit a **different**, related failure: a throwaway probe program
(`pdump.a`, not the final deliverable) had a real bug — a subroutine call
inside a loop clobbered the register the loop was using as its counter,
causing an unbounded write that overflowed the module's scratch data area
and hung the emulated CPU. `send "pdump ABCDEFGHIJKLMNOP"` returned
`[TIMEOUT]` with zero output, and a follow-up probe command (`dir`) also
timed out — a genuinely wedged guest, not a stale prompt. The skill's own
mitigation advice ("send one cheap probe... if it comes back stale, go
straight to `restart`, don't keep retrying blind") is exactly what I did,
and `restart` recovered cleanly in the normal ~40s boot window. This
matches the skill's documented "after a long computation" gotcha in
spirit, though my trigger was an infinite loop bug, not a long
legitimate computation — worth noting the mitigation generalizes fine
either way.

## Overall: substitute for knowing 6809/OS-9, or real gaps?

Not a substitute, and I don't think it's meant to be one. The skill got
the *OS-9 conventions and toolchain* layer right and fast: which
assembler, its exact CLI syntax, the syscall dispatch mechanism, the
specific codes needed, module-header mechanics well enough to fix once
shown the discrepancy, and (critically) working debugger commands with
accurate documented behavior. All of that saved real time and prevented
several classes of mistake I'd otherwise have made blind (especially the
`rma` dead end).

What it can't do, and doesn't claim to: catch 6809-CPU-semantics bugs in
*my own* register-lifetime reasoning (the `LDD`-clobbers-digit-counter
bug), or definitively resolve a genuinely unsettled OS-9 fact that
apparently differs by invocation path (the parameter-register leading-
space question) without more live investigation than a single session
affords. Both of those needed the debugger and direct reasoning about
what the CPU was actually doing, not a documentation lookup. A user who
doesn't already know how to read a 6809 register dump, set a breakpoint,
and reason about what "`LDD` touches all of `D`" implies for an
in-progress loop would have been stuck at the "prints garbage, doesn't
crash" stage with no assembler diagnostic to point the way — the skill's
debugger-command documentation makes that investigation *possible*, but
someone still has to know to look there and how to interpret what comes
back.
