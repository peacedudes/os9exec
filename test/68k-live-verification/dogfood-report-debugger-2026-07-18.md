# Dogfood report: live symbolic debugging with the OS-9 `debug` command (2026-07-18)

Task: write a hand-written 68k assembly program with a deliberate,
non-obvious logic bug, then actually find it using the real OS-9
`debug` command on live os9exec via `tools/os9repl.sh` -- breakpoints,
single-step, register/memory inspection -- rather than by re-reading the
source. Mirrors an earlier same-day 6809 dogfood pass that found a real
register-clobbering bug purely through breakpoints and register
inspection. Working source (final, fixed, with the bug documented in
its header): `dogfood-debugger-signextend-sum.a`.

The bug: `sumstats` sums ten signed-byte values into a long accumulator.
The intended widening was `ext.w d0` (sign-extend byte->word) then
`ext.l d0` (word->long). The buggy version used `and.w #$00FF,d0`
instead of `ext.w d0` -- a very natural-looking "mask off the byte"
idiom that zero-extends instead of sign-extends, silently turning every
negative delta into a large positive number (`-3` read back as `+253`)
while a separate, unaffected negative-count stat kept printing correctly
-- exactly the kind of "one number's right, one's wrong" signal meant to
require tracing, not spotting.

## Debugger commands used, and how they behaved

- **`r68 -O=<name>.r <name>.a`, `l68 -g -O=<name> <name>.r`** to build
  with debug symbol data, `debug <name>` to enter. Matches the skill's
  `cc -g` recipe in spirit, but `r68`/`l68` need their own flags
  (`-O=` for object output, `-g` for the debugger symbol module) --
  the skill's debug section states the `cc -g` case and doesn't cover
  assembling by hand, so this took a `help r68`/`help l68` detour to
  find (both gave full, accurate option lists live).
- **First real finding, and it directly resolves a disclosed skill
  gap**: with plain colon-less labels (`start`, `sumloop`, ...), `debug`'s
  `sc` showed only three symbols total (`btext`/`bname`/`etext` -- the
  module's own boundary markers), none of the program's own labels.
  `68k/os9-68k-assembly.md`'s "Known gaps" flags this outright as
  untested: *"The external-symbol 'trailing colon = public' visibility
  convention."* Adding a trailing colon to the labels I wanted visible
  (`start:`, `sumloop:`, `notneg:`, `printnum:`) and reassembling made
  all four appear correctly in both `l68 -s`'s linkage map and `debug`'s
  `sc` listing, at their real addresses. **This resolves that gap
  live**: colon-suffixed labels are exported/debugger-visible on this
  `r68`/`l68`; colon-less labels are pure local assembler symbols and
  invisible to both the linker's map and the debugger. Worth folding
  into `os9-68k-assembly.md` -- flagged here per the task's
  diagnostic-only instruction, not fixed directly.
- **`b <name>` + `g`** -- worked exactly as documented, no surprises.
  Used two breakpoints (`sumloop`, the loop top; `notneg`, reached every
  iteration regardless of sign, right before the buggy code) to
  bracket each loop pass cleanly.
- **`gs`** -- used only for genuinely straight-line stretches between
  breakpoints (stepping `move.b`/`tst.b`/`and.w`/`ext.l`/`add.l` one at
  a time), never across the loop's own conditional branches. The skill's
  warning that `gs` on a taken branch runs to the *fall-through*
  address (possibly a whole extra loop iteration) rather than doing a
  true single-step is accurate and worth respecting literally -- the
  `b`+`g`-to-bracket, `gs`-inside strategy never needed a Ctrl-C
  recovery.
- **Register inspection** -- `debug` auto-prints the full `dn:`/`an:`/
  `pc:`/`cc:` block after *every* `g`/`gs` stop, not just on explicit
  request. This made the documented bare `.` command feel almost
  redundant in practice (tried once explicitly, output identical to the
  auto-print already on screen) -- genuinely helpful, not a skill gap,
  just worth noting that a session rarely needs to ask for registers on
  purpose.
- **`di <addr> <n>`** -- `di sumloop 6` disassembled six instructions
  without executing, matched the source exactly instruction-for-
  instruction, and rendered `and.w #$00FF,d0` as `and.w #255,d0`
  (decimal, not hex) -- a harmless but slightly confusing default worth
  knowing about when cross-checking a hex immediate by eye.
- **`sc` (bare)** -- listed all real code symbols cleanly once the
  colon fix was applied; never tried `sc <module>` (already documented
  as broken) or the flagged double-relocated address bug, since nothing
  in this session needed an address computed from `sc`'s output.

## The actual find

At the `notneg` breakpoint for the second array element (delta = -3),
the register dump showed `d0=$000000FD` (253) and `d1=$00000005` (5,
the correct running sum after the first, positive element). Stepping
`gs` through `and.w #$00FF,d0` left `d0` unchanged at `$000000FD`;
stepping through the following `ext.l d0` *also* left it unchanged --
itself proof the extend was inert, since a genuine sign-extend of a
byte with its top bit set must produce upper bytes full of 1s
(`$FFFFFFFD`), not stay `$000000FD`. The next `add.l d0,d1` then
produced `d1=$00000102` (258 = 5+253) instead of the correct
`d1=$00000002` (5-3=2) -- the exact instruction the bug lives in, pinned
down by watching the register transcript change (or fail to change) at
each step rather than by reasoning about the code in the abstract.

## Bottom line

The documented debugger commands (`b`, `g`, `gs`, `.`, `di`, `sc`) all
behaved exactly as `using-os9exec-repl.md` describes, including both of
its explicit warnings (`gs` is fall-through-not-single-step; `sc`'s
addresses are unreliable). The one real gap found and resolved live was
the assembler/linker side, not the debugger itself: colon-less labels
never reach the debugger's symbol table at all, which the skill already
flagged as an open question -- this session answers it (trailing colon
= public/debugger-visible, confirmed via `l68 -s`'s linkage map and
`debug`'s `sc`).
