# Dogfood report: 68k syscall live-verification batch 2 (2026-07-20)

Continues batch 1. Module/process lifecycle: `F$Load`, `F$Link`,
`F$UnLink`, `F$Fork`, `F$Wait`, all in one file
(`test/68k-live-verification/batch2-01.a`) exercising `childprg68k.a`
(a trivial `F$Exit(77)` module built in batch 1) as the load/link/fork
target throughout.

## All 5 confirmed cleanly, first attempt after the register-discipline fix

Checked `os9exec`'s own `OS9_F_Load`/`OS9_F_Link`/`OS9_F_UnLink`/
`OS9_F_Fork`/`OS9_F_Wait` (`Source/OS9exec_core/fcalls.c`) before writing
any test code, per the batch 1 methodology finding — all five matched
`68k/syscall-reference.md`'s already-documented register layout exactly,
including `F$Fork`, whose row previously carried a `Flag` for
conflicting register-layout claims across different manuals. That flag
is now resolved: `os9exec`'s real implementation matches this skill's
existing table, so the conflicting manual passages were simply wrong for
this call (or describe something else).

Applied the batch-1 `F$CmpNam` lesson proactively this time — every
register that feeds a `.w`-documented syscall parameter is fully cleared
first (`moveq`/`clr.l`, never a bare `move.w`/`clr.w` into a register
that held a full longword earlier in the same program). Assembled clean
on the first pass with zero register-discipline bugs, a direct payoff of
writing that gotcha into the skill file after batch 1 rather than just
noting it once and moving on.

Live output:

```
PASS F$Load type=1
PASS F$Link (already resident via F$Load)
PASS F$UnLink x2 (one per link)
PASS F$Fork pid=7
PASS F$Wait pid/status=7
 77
```

`F$Wait`'s reported exit status (`77`) is an exact match for
`childprg68k.a`'s own `F$Exit(77)` — end-to-end confirmation that the
forked child is really the intended module executing, not just a
process that happened to exit.

`F$Load`'s reported type (`1`) wasn't traced further — plausibly just
the raw type nibble (`Prgrm`=1) rather than the full packed
`Type_Lang` word; not chased, doesn't affect the pass/fail confirmation.

## Toolchain note (same class as batch 1, not a new gotcha)

`F$Load("childprg68k")` initially failed `E$PNNF`/216 — not a syscall
bug, a missed setup step: `childprg68k.a` had only been *assembled*
(`r68`) in batch 1, never *linked* (`l68`) into an actual loadable
module file. `F$Load` needs a real module on disk, not just a `.r`
relocatable object. Fixed by linking it first. Re-confirms the
account's `chx`-vs-`chd` ambiguity already noted in batch 1 and the
2026-07-18 filemgr session: `l68 -o=<bare-name>` output is invisible to
`dir` (which lists the data directory) but still resolves correctly by
name (command lookup and `F$Load` both search the execution directory).

## Files

- `test/68k-live-verification/batch2-01.a` — F$Load/F$Link/F$UnLink/F$Fork/F$Wait
