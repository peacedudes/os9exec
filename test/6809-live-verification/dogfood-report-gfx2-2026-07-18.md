# Dogfood report: GFX2 install + GOSET/PALETTE live verification (2026-07-18)

Task: get real GFX2 support onto the live NitrOS-9 (6809) test disk (the
module was missing on this project's EOU test image, blocking an earlier
attempt -- see `VERIFICATION-BACKLOG.md` item 2's "2026-07-18 attempt"
note), then use it to settle two previously-`Manual`-only open questions
in `6809/gfx-windowing.md`: `GOSET`'s real syntax, and `PALETTE`'s
register-number range. Source used: this project's own copy of the
community-maintained (Tandy-derived, openly redistributed) `gfx.asm` /
`gfx2.asm` package at
`os9/nitros9/source/3rdparty/packages/basic09/`, building and using it
locally per the project owner's explicit authorization -- not
redistributing it, not first-publishing any proprietary fact.

Mechanical how-it-was-done detail (ToolShed injection commands, the four
asm/MIA porting patches, a real ToolShed bug found along the way) is in
`dogfood-gfx2-install-notes.md`. The literal BASIC09 test variants run are
in `dogfood-gfx2-goset-palette-test.bas`. This file is the narrative
summary and the answers.

## Did the build/install work?

**Yes, fully.** Both `gfx` and `gfx2` assembled with `asm` (not `rma`,
per the existing "rma hangs" finding -- never attempted) with `00000
error(s)` each, loaded into `CMDS` with good CRCs (`ident` confirmed),
and ran live from BASIC09.

Getting there took four rounds of real, non-obvious fixes, all found by
iterating actual `asm` error output rather than guessing:

1. Comment-leader mismatch (`;` vs. `*`) -- both files were written for
   the modern `lwasm`-based NitrOS-9 build, not classic `asm`/MIA.
2. Three genuine 8-character label-truncation collisions in `gfx.asm`
   (`asm`/MIA's label limit is 1-8 chars; the source's real labels are
   longer and collide once truncated) plus one true duplicate local-style
   label (`ret@` used twice, unscoped) -- `asm`/MIA has no local-label
   scoping the way the modern toolchain apparently allows.
3. `gfx2.asm`'s `H6309` conditional-assembly symbol, normally supplied by
   `lwasm -DH6309=...` at build time, is genuinely undefined under
   `asm`/MIA (no `-D` equivalent) -- caused a real cascade of
   "undefined name"/"phasing" errors until fixed with an explicit
   `H6309 set 0` (correct choice: this is a 6809, not 6309, disk).
4. Three individual source lines in `gfx2.asm` (135-136 characters) hit
   what appears to be an undocumented `asm`/MIA per-line length limit
   somewhere between 132 (fine) and 135 (breaks) characters -- fixed by
   moving the overflow comment text to its own `*` line.

All four fixes are mechanical/behavior-preserving (comment-leader
translation, symbol renaming, an explicit default for an already-implied
0 value, comment relocation) -- no logic was changed. A real ToolShed
(`os9` CLI) bug was also found and worked around along the way: `copy -r`
(rewrite-in-place) can leave stale bytes from a file's *previous*
occupant of the same disk segments even though the reported file size is
correct; `del` then a fresh `copy` fixed it. See the install-notes file
for the full repro.

## GOSET: real syntax

**`GOSET` does not exist as a real, callable GFX2 function in this
build.** `Live`, high confidence, two independent confirmations:

- **Runtime**: `RUN GFX2("GOSET",1)` from a compiled BASIC09 procedure
  gives `Error #048 -- Unimplemented Routine` and BASIC09 drops into its
  own debugger (`D:`) -- this is GFX2's own function-name dispatch table
  reporting "not found" (`E$NoRout`), the same mechanism that produces
  `NoFunc`/`E$NoRout` in `gfx.asm`'s own source for an unrecognized
  function name.
- **Source**: `grep -n "GOSET" gfx2.asm` (case-sensitive and
  case-insensitive) returns nothing; a full listing of every `fcc`
  string in the function table (~60 entries -- `Mouse`, `Point`, `Line`,
  `Box`, `Bar`, `PutGC`, `Fill`, `Circle`, `FCircle`, `DWSet`, `Select`,
  `OWSet`, `OWEnd`, `DWEnd`, `CWArea`, `DefBuff`, `KillBuff`, `GPLoad`,
  `Get`, `Put`, `Pattern`, `Logic`, `DefCol`, `Palette`, `Color`,
  `Border`, `ScaleSw`, `DWProtSw`, `GCSet`, `Font`, `TCharSw`, `BoldSw`,
  `PropSw`, `SetDPtr`, `Draw`, `Ellipse`, `FEllipse`, `Arc`, `CurHome`,
  `CurXY`, `ErLine`, `ErEOLine`, `CurOff`, `CurOn`, `CurRgt`, `Bell`,
  `CurLft`, `CurUp`, `CurDwn`, `ErEOWndw`, `Clear`, `CrRtn`, `ReVOn`,
  `ReVOff`, `UndlnOn`, `UndlnOff`, `BlnkOn`, `BlnkOff`, `InsLin`,
  `DelLin`, and more) has no `GOSET` entry anywhere.

This matches the manual corpus's own weak signal (`GOSET` mentioned
exactly once, in passing, in a `PUTGC` worked example, with no formal
entry anywhere) and upgrades it from "undocumented" to "confirmed absent
from the real implementation" -- i.e. the manual's own worked example is
itself unreliable/aspirational on this point, not just under-documented.
Whatever `PUTGC`'s real graphics-cursor-shape-selection mechanism is (if
any exists beyond `GCSET`, which *is* real and in the table), it is not
`GOSET` in this build.

## PALETTE: register-number range

**Not bracketable the way originally planned -- and that's itself the
finding.** `Live`. The plan was to find what OS-9/GFX2 rejects vs.
accepts to locate the real boundary. In practice, nothing tested was
rejected:

- `RUN GFX2("PALETTE",5,10)` (in-range guess, no path) -- ran clean.
- `RUN GFX2("PALETTE",20,10)` -- ran clean.
- `RUN GFX2("PALETTE",99,10)` -- ran clean.
- Same register-99 call routed through an explicitly-opened, real window
  path (`OPEN #p,"/W1":WRITE` then `RUN GFX2(p,"PALETTE",99,10)`) --
  still ran clean, ruling out "the no-path default target isn't a real
  window anyway" as an alternative explanation for the silence.
- `RUN GFX2(p,"PALETTE",5,200)` (color 200, far outside the documented
  0-63/64-color range) against the same real `/W1` window -- also ran
  clean.

Reading `gfx2.asm`'s own `PALETTE` handler (label `L0585`) confirms why:
it only checks the BASIC09 *parameter count* (must resolve to exactly 3
arguments including the function-name string) before packaging
register+color into an escape-code byte stream and writing it via
`I$Write` -- there is no source-level bounds check on either value in
this client module. And nothing downstream rejected the out-of-range
values either, at least as far as an OS-9 error return is concerned.

**So the confirmed fact is:** this GFX2 build performs no software range
validation on `PALETTE`'s register or color arguments, at least up to
the values tried (register 99, color 200) -- not that "0-15 is the real
range," which remains unconfirmed and is now known to be
*unenforced* rather than silently-validated. What actually happens on
real CoCo3 GIME hardware when an out-of-range register/color is written
(silent wraparound via address aliasing, corruption of adjacent
driver/window state, or something else) was not established here --
that needs actual pixel/palette observation, which this session did not
attempt (see below) and which a text-only REPL channel can't provide
regardless.

## Optional bonus part (actual render / screenshot)

**Not attempted.** The module-install and GOSET/PALETTE work above
already represented real, non-trivial effort (four separate porting
fixes plus a genuine ToolShed bug), consistent with the task's own
"don't let the bonus balloon the session" guidance. Getting a real XRoar
GUI window open (`-ui macosx`) and driving a `DWSET`/drawing-primitive
call plus a screenshot per `using-nitros9-repl.md`'s GUI-screenshot
section remains a legitimate future follow-up, now unblocked (GFX2 is
installed and loadable on this disk going forward, since the disk image
persists across restarts).

## Files

- `dogfood-gfx2-install-notes.md` -- ToolShed commands, the four asm/MIA
  source patches, the ToolShed segment-corruption bug and its workaround.
- `dogfood-gfx2-goset-palette-test.bas` -- the six BASIC09 test variants
  actually run, with each one's literal live result.
- `6809/gfx-windowing.md` and `VERIFICATION-BACKLOG.md` (in
  `~/.claude/skills/os9-dev/references/`) updated directly with these
  findings, `Live`-tagged, per this task's instructions.
