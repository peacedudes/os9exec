# Dogfood report: hand-written 68k assembly, raw I$ syscalls (2026-07-18)

Task: write a 68000 assembly-language OS-9 program (a proper module, via
`r68`/`l68`, not BASIC09 or C) that opens a file named from the raw
command-line parameter area using raw I$ syscalls, counts lines/chars
(excluding line terminators), prints a summary, and exits cleanly --
handling file-not-found gracefully -- using only the `os9-dev` skill plus
live testing via `tools/os9repl.sh`. Working source is
`test/68k-live-verification/dogfood-asm-line-counter.a`, with its own
header comment giving the literal verified output.

This specifically stress-tests `68k/os9-68k-assembly.md`, whose "Known
gaps" section explicitly flags `psect`/`vsect`/`csect` directive syntax
as **unverified for 68k** ("the documented PSECT/VSECT syntax is the
6809 RMA's") -- a real, disclosed hole, not a confident-but-wrong claim
-- and was rewritten today in a redundancy pass to cross-reference
`common/module-format.md` instead of restating module-header material.

## Did the skill get me there first-try, or did I have to guess/experiment?

No -- this was a genuine reverse-engineering exercise, and the skill was
upfront that it would be. Total: 8 small throwaway probe programs plus
2 full iterations of the real program, roughly 13 assemble/link/run
cycles altogether. Unlike the C dogfood pass earlier today (2 failed
compiles, then clean), this one had no working syntax to start from at
all for two separate unknowns: the assembler's own directive syntax, and
the live register-level parameter-passing contract. Concretely:

**1. `psect` syntax -- the skill's disclosed gap was accurate, but the
fix was findable, just not where the skill points.** `os9-68k-assembly.md`
now says (post-trim): *"Full header layout/offsets... `common/
module-format.md` -- not repeated here... an assembly-language program's
`psect` declaration is what populates these fields (even though, per
'Known gaps' below, the exact `psect` directive syntax isn't attested in
this source set)."* I followed that cross-reference into
`module-format.md`, which is honest about the same gap ("no root psect
found... though the linker takes it from whichever psect was designated
root" -- describes the *effect*, never the syntax). Neither file's
"Cross-references" section points at the one place in the whole skill
that actually has working syntax: `basic09/basic09-vs-68k-differences.md`'s
"Calling 68000 machine-language procedures from BASIC09" section, which
contains a complete, already-`Live`-tested hand-written 68k assembly
example (`psect addone,Type_Lang,Attr_Rev,0,0,addone`) with real
`r68`/`l68` invocations. I only found it because I searched the whole
skill tree for the literal string `psect`, not because either file
pointed me there. **This is the single most actionable, fixable finding
in this report**: `os9-68k-assembly.md`'s "Known gaps" and
"Cross-references" sections should point at that BASIC09 file's worked
example -- it's the skill's only live-verified 68k assembly source,
and it's currently undiscoverable by cross-reference alone.

Once found, the 6-operand shape
(`name,typelang,attrrev,edition,stacksize,entry`) worked immediately for
a `Prgrm`-type module too, not just the `Sbrtn` it was demonstrated with
-- a genuine cross-file win once located.

**2. No I$/F$ call-code equates exist anywhere on this disk image.**
First real error, after getting `psect` right:
```
Symbol 'I$Write' unresolved.
	Referenced by psect 'probe' in file 'probe.r'
Symbol 'F$Exit' unresolved.
	Referenced by psect 'probe' in file 'probe.r'
l68: error - unresolved references
```
`syscall-reference.md`'s own documented pattern (`trap #0` / `dc.w
I$Close`, or the `OS9 I$Close` pseudo-op) assembles cleanly either way --
`r68` treats an unknown name after `dc.w` as an ordinary external symbol
and defers resolution to the linker, so the assembler gives no warning at
all. It's only `l68` that reveals nothing on this disk defines what
`I$Write` numerically *is*. I searched all of `/h0/DEFS` (`oskdefs.d`,
`macros.d`, both `os9lib`/`GCC2` header trees) for the literal string
`I$Open` -- zero hits anywhere. The skill doesn't claim such a file
exists, so this isn't a wrong claim, just an unstated toolchain fact
worth a line: **on this SDK, `syscall-reference.md`'s call names are not
directly assemblable without hand-defining the numeric codes yourself.**
Fixed by hand-defining them from this project's own `os9funcs.h`
(`Source/OS9exec_core/os9funcs.h` -- the project's own reimplementation,
not proprietary Microware source, and confirmed to match the standard
published call-code table): `IOpen=$84, IRead=$89, IWrite=$8A,
IClose=$8F, FExit=$06`.

**3. The register-conventions table's D5="param-area size" claim did not
behave as a plain byte count in live testing** -- and neither
`os9-68k-assembly.md` (which already hedges this exact table with "Slot
assignments... vary between manual passages... confirm an exact slot
against a live register dump") nor anywhere else in the skill documents
what the parameter area's *content* actually looks like. A raw
`I$Write` from A5 using D5 as the count produced garbled, seemingly
environment-variable-like text with no visible trace of the typed
argument -- the actual root cause (worked out over several probes, see
below) was that the buffer legitimately starts with `"hello"` but a bare
CR three bytes later was resetting the terminal's cursor to column 0,
visually erasing it before I sanitized the dump. This is worth flagging
as a live-testing methodology note as much as a skill gap: **raw
byte-dumps of OS-9 process memory to a terminal need control-byte
sanitizing before they can be trusted by eye.** The practical, sufficient
fact this session established (not previously in the skill anywhere):
**A5 at program entry is a NUL-terminated string holding exactly the
typed command tail** (`"hello"` for one argument, verified via hex dump:
`68 65 6C 6C 6F 00 0D 00 50 4F 52 54...` = `"hello"` + NUL + CR + NUL +
`"PORT="...` (environment variables follow)), safe to pass directly as
`I$Open`'s pathname pointer with no copying needed. **With zero
arguments, the first byte is a bare CR (`0x0D`), not a NUL** -- confirmed
via the same hex-dump technique (`0D 00 50 4F 52 54...`). My first
implementation checked only for a leading NUL and got this wrong in a
way that only showed up at runtime, not assembly/link time: running
`linecount` with no arguments incorrectly fell into the file-open error
path (`cannot open file: `, with an empty name) instead of printing the
usage message, because the "empty" check missed the CR case. I never
determined what D5's exact value legitimately represents (possibly the
size of the whole parameter+environment block, not just the argument
text) -- I didn't need to, once NUL-termination gave me a sufficient,
simpler signal, but I want to be precise that I'm not calling the
existing claim definitively *wrong*, only unverified-in-a-way-that-cost-
real-debugging-time and not actionable as documented.

**4. One pure-68k-ISA error, out of the skill's stated scope but worth
noting for completeness:** `subq.l #NBYTES,a7` / `addq.l #NBYTES,a7`
with `NBYTES set 48` failed with `*** error - value out of range ***` --
`SUBQ`/`ADDQ` only accept immediate values 1-8 on real 68000 hardware;
fixed by using `sub.l`/`add.l` instead. The skill correctly disclaims
the 68000 instruction set as out of scope ("sources assume Motorola
knowledge"), so this isn't a gap, just a real error hit along the way.

## Was anything actually wrong (not just missing) in the skill?

One thing crossed from "gap" to something closer to actively
counterproductive if trusted at face value without live-testing: the
D5="param-area size" register-table entry, in the specific sense that a
reader who trusted it as "the byte count to bound a raw memory
read/write with" -- a completely reasonable reading of "size" -- would
reproduce the exact garbled-output confusion this session hit. The skill
does already hedge this table generally ("confirm an exact slot against
a live register dump before depending on it"), so it isn't an
unqualified false claim, but the hedge doesn't warn about *this specific
failure mode* (reading past a NUL-terminated string using a size that
turns out not to bound just that string).

Everything else that was "wrong" was really "silently absent, not
false": no I$/F$ numeric call codes anywhere on the disk despite
`syscall-reference.md` presenting call names as directly usable, and no
cross-reference from `os9-68k-assembly.md`'s own disclosed `psect` gap to
the one file in the skill that actually resolves it.

## Was anything surprisingly helpful?

- **`l68`'s error taxonomy really is diagnostic gold**, exactly as
  `module-format.md` claims. `Symbol 'I$Write' unresolved... Referenced
  by psect 'probe' in file 'probe.r'` pointed at the exact right cause
  (missing numeric definition, not a syntax problem, not a wrong module
  type) on the first try, with zero ambiguity.
- **The BASIC09 file's worked psect example, once found, transferred
  perfectly to a completely different module type** (`Prgrm` vs. the
  demonstrated `Sbrtn`) with zero changes needed to the directive shape
  itself -- a real payoff from a previous session's `Live` verification
  work, even though it wasn't the file I expected to need for this task.
- **`syscall-reference.md`'s carry/`d1.w` error convention and the
  `trap #0` / `dc.w <code>` dispatch pattern worked exactly as
  documented**, immediately, once real numeric codes were in hand --
  `bcs.s noopen` / `bcc` behaved precisely as described for every one of
  `I$Open`/`I$Read`/`I$Close`.
- **`using-os9exec-repl.md`'s `flip -m` file-creation route** continued
  to work flawlessly for both source files and ad-hoc test-input files
  across a dozen-plus iterations, exactly as the earlier C dogfood pass
  also found.

## Did I hit any REPL sub-prompt or harness friction?

No BASIC09/`vi`/`debug` sub-prompt friction -- never left the ordinary
shell prompt. But real friction of a different kind: **the gated `send`
lost sync with the shell prompt at least twice**, both times right after
a child program wrote a bare CR (no LF) to the terminal mid-output. The
CR repositioned the cursor to column 0 without clearing the line, so the
tmux pane's last "line" ended up with `claude:` (a genuine fresh prompt)
sitting in the *middle* of a row of leftover garbled text rather than at
its start -- `send`'s prompt-matching apparently wants the prompt at the
start of a line, so it just timed out (`[TIMEOUT waiting for prompt]`)
without actually failing to deliver the command underneath. Recovery was
reliable and cheap: one raw `key Enter` to force a fresh blank prompt
line, confirmed via `snap`, then resend the command that had appeared to
time out (it had, in fact, never been sent, so nothing was lost). Once
my own program stopped emitting bare CRs (switched to CR+LF), this
stopped happening on later runs. Worth a line in
`using-os9exec-repl.md`'s "two send modes" section: a program's own raw,
unedited output containing a bare CR can desync the gate the same way an
interactive sub-prompt does, and the fix is the same (`key Enter`, then
resend).

The debugger (`debug`) was never needed -- once the parameter-area
format was pinned down via hex-dump probes and the string-quoting issue
was fixed, the real program worked correctly on the first full run for
the happy path, and needed exactly one logic fix (the CR-vs-NUL
empty-argument check) for the error path.

## Overall: substitute for knowing 68k assembly/OS-9, or real gaps?

For this task, the skill was **not** a substitute for knowing 68k
assembly and OS-9's assembly-level ABI -- it correctly told me so in
advance (the disclosed `psect` gap), and the actual work of pinning down
`psect` syntax, the I$/F$ numeric codes, and the parameter-area format
was all reverse-engineering against the live toolchain, not skill
lookup. But the skill was honest about exactly where its coverage ended,
which meant no time was lost chasing a confidently-wrong claim -- every
real gap hit was a disclosed one (`psect` syntax, register-slot
uncertainty) except the two genuinely new findings this pass surfaced
(no I$/F$ defs file exists on this disk; D5 doesn't bound just the
argument text). The syscall-level material that *was* documented
(`syscall-reference.md`'s carry convention, call semantics, module
header/CRC mechanics in `module-format.md`) held up perfectly once I had
real numeric codes and real directive syntax to combine it with. The
single highest-value fix for next time: make `os9-68k-assembly.md`'s
"Known gaps"/"Cross-references" sections point at
`basic09-vs-68k-differences.md`'s worked example instead of leaving a
reader to discover it by a whole-skill grep, the way I had to.
