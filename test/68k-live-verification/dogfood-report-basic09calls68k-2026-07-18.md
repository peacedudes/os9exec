# Dogfood report: BASIC09 calling 68k assembly, beyond the minimal example (2026-07-18)

Task: write a small BASIC09 program that calls a hand-written 68k assembly
subroutine to do something a bit more substantial than the existing
minimal worked example in `basic09/basic09-vs-68k-differences.md`
("Calling 68000 machine-language procedures from BASIC09"), using only
the `os9-dev` skill plus live testing via `tools/os9repl.sh` at full
speed (`start -r`). Working source is
`test/68k-live-verification/dogfood-basic09calls68k-addmulmod.a` (the
68k assembly Sbrtn module) and
`dogfood-basic09calls68k-calctest.bas` /
`dogfood-basic09calls68k-calctest-negative.bas` (the two BASIC09 test
programs), each with its own header comment giving the literal verified
output.

The existing skill example (`addone`) is a single by-reference INTEGER
parameter incremented in place -- enough to prove the mechanism exists,
but not enough to exercise the stack-based parameter area at all (only
`D1` is ever touched). This task's `addmulmod(a,b,sum,prod)` uses four
parameters -- `a` by reference, `b` by value (a literal), `sum`/`prod`
by-reference outputs -- specifically to exercise the documented
"4-byte length + 4-byte address per parameter after the first, pushed
above the return address" stack layout, and the by-value-vs-by-reference
distinction the skill describes but never demonstrates.

## Did the skill get me there first-try, or did I have to guess/experiment?

**First try, no iteration at all.** Assemble (`r68`), link (`l68`), and
both BASIC09 runs (`calctest`, `calctest-negative`) each succeeded on the
very first attempt -- zero assembly errors, zero link errors, zero
crashes, zero wrong-value logic bugs. `sum`/`prod` came back correct
(13/42, then 7/-30 for the negative-operand cross-check) the first time
the program ran. The only real friction in the whole session was
REPL-harness mechanics unrelated to the skill's technical content (see
below), not anything about the calling convention itself.

This is a sharp contrast with the same skill file's earlier `asm`-vs-raw-
I$-syscalls dogfood pass today, which needed ~13 assemble/link/run
cycles to reverse-engineer `psect` syntax and numeric I$/F$ call codes
from scratch. The difference: that task had no prior worked example at
all, while this one started from the `addone` skill example, which
turned out to transfer perfectly to a materially different case (4
params instead of 1, mixed by-value/by-reference, real arithmetic
instead of a bare increment) with zero changes needed to the mechanism
itself -- only the obvious extension of reading more of the documented
stack layout.

## Was anything in the skill's psect/calling-convention guidance wrong, missing, or confusing?

Nothing wrong. One thing worth flagging as **under-illustrated rather
than incorrect**: the stack-parameter-layout paragraph states the shape
in prose ("a 4-byte length followed by a 4-byte address, in call order,
going toward higher addresses") but the skill's only worked example
never actually reads a stack parameter -- `addone` has exactly one
parameter, entirely satisfied by `D1`. A reader has to compute the
concrete offsets (8(a7)/16(a7)/24(a7) for parameters 2/3/4, each address
field sitting 4 bytes past its own length field) from the prose alone,
with nothing to check the arithmetic against. It happened to be right on
the first try here, but that's a "the prose was precise enough to derive
correctly," not "there was a worked multi-parameter example to copy
from." Given how cheap this dogfood pass was, the addmulmod source
written here would be a reasonable second worked example for the skill
to eventually point to if multi-parameter calls come up again -- flagging
for review, not doing it myself per the task's scope.

The by-value/by-reference paragraph is correct and was genuinely useful
in advance: it explicitly says both cases hand the routine an address
either way, just of different storage. That's exactly what happened --
passing the literal `7` for `b` worked identically to passing the
by-reference `a`, no special-casing needed in the assembly.

The `Sbrtn`/`Objct`/`ReEnt` psect example continues to transfer cleanly
-- this is now confirmed for a `Sbrtn` module with a genuinely different
shape (more params, real arithmetic) beyond the two other module types
(`Prgrm`/`Drivr`) already confirmed transferable in an earlier session
today.

## Was anything surprisingly helpful?

- **The already-documented `l68 -o=` writes to `chx` not `chd` gotcha**
  (in `using-os9exec-repl.md`) predicted exactly what happened: the
  linked `addmulmod` module didn't appear in the account's home directory
  after linking, and was found in `/dd/CMDS/DOG` (the account's `chx`)
  without needing to search or guess -- a direct, correct prediction from
  a previous session's finding, saving what would otherwise have been a
  short debugging detour.
- **`ident`'s header-field dump** (`Ty/La $201`, `At/Rev $8001`, "68000
  Sub Mod, Object Code, Sharable") gave an exact, unambiguous confirmation
  that the `psect` line's six operands landed in the right header fields
  -- cheap and conclusive, worth reaching for by habit whenever a new
  `psect` invocation needs a sanity check before running it.
- **The `math` gotcha's stated fix worked prophylactically**: `load math`
  (and `load basic`) before touching BASIC09 meant the documented
  "Can't install trap handler" false-lead never had a chance to appear,
  even though this program's `DIM ... INTEGER` declarations were exactly
  the kind of numeric handling that triggers it.

## Did I hit any REPL sub-prompt or harness friction?

Yes, one self-inflicted issue, not a skill or harness bug: the very
first `key` send for the assembly file's first line
(`key "\ nam addmulmod" Enter`) included a stray literal backslash from
an over-escaped shell quoting attempt, which landed in the OS-9 `tee`
session as typed text. Recovery followed the documented pattern exactly
(`key Escape` to abort `tee`, confirmed via `snap`, `del` the partial
file, restart `tee` clean) -- cheap, and exactly as
`using-os9exec-repl.md` describes for a garbled sub-program session. No
gated-`send` desync this session (no bare-CR-only output from any
program under test, unlike the earlier `asm` dogfood pass today).

## Overall

For a task specifically framed as "does the calling mechanism generalize
beyond the minimal example," the answer is an unambiguous yes, and the
skill supported it well: the one worked example, combined with the prose
description of the stack-parameter layout and the by-value/by-reference
note, was sufficient to write a correct 4-parameter, mixed-passing-mode,
real-arithmetic routine with zero iteration. The only actionable
follow-up is minor: consider eventually adding a multi-parameter worked
example (this session's `addmulmod` is a ready-made candidate) so a
future reader has something to check hand-derived stack offsets
against, rather than deriving them from prose alone -- flagged for
review per the task's boundaries, not applied here.
