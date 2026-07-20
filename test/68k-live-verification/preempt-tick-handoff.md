# `-q` pre-emption: the r68 crash, and why the first theory was wrong

Resolved 2026-07-19. Kept because the wrong answer here was expensive twice,
and the shape of the mistake is worth more than the fix.

## The bug

With `-q`, `r68` died of a bus error part-way through assembling, so the two
`F$STrap` tests never got their program built. Read the failure as "the
assembler crashed", not "the handler missed" -- exception handling was never
involved.

**Root cause.** In `handle_os9exec_exception()` (`os9_uae.c`), the TCALL branch
set `m68_os9go_result = (nr<<16)+func` *before* checking whether a trap handler
was installed. When one was, the emulator jumps into the handler and KEEPS
RUNNING -- but that prepared token stayed live. A tick can stop the loop at any
instruction inside the handler, so os9exec was handed a TCALL it had already
dispatched and built the stack frame a second time on a process already inside
its handler. Fix: set the result in the no-handler branch only. A result only
means something if we are actually leaving.

Same shape as the earlier F$Link bug (`57bf202`), and worth remembering as a
class: *state prepared for an exit that then does not happen is a live grenade
once something else can end the loop.*

## The theory that was wrong

The first handoff said the round trip through `regs_type` loses CPU state, and
named `regs.prefetch`. It is wrong. Measured, do not re-derive it:

- Snapshot `regs` at tick exit, byte-compare on re-entry: **zero differing
  bytes** over 15 real comparisons (only `usp`, which `llm_os9_go` deliberately
  reloads from `a[7]`).
- Tick that never leaves `m68k_os9go`: still crashed (2/8).
- Signal armed but `os9_running` never cleared: crashed 0/8.

So the damage needed the loop to STOP, not the round trip to lose anything.
`fill_prefetch_0()` "taking a test from 0/6 to 6/6" was timing noise -- which is
exactly how a confident wrong theory gets built from a real-looking number.

## System state: the guard that never fires

`m68k_os9go()`'s `if (regs.s) defer` has never once fired -- 167 of 167 ticks in
a suite run had `s=0`, and nothing sets the supervisor bit for guest execution.

That is not a bug. OS-9's rule is that a tick cannot pre-empt a process inside a
system request but may pre-empt a user-state process to wake a system one.
OS9exec satisfies the first half structurally: a system request traps OUT of the
emulator and runs as host C while the loop is not executing, so the only place a
switch can be decided is already user state. Do not read the `regs.s` test as
what protects system calls, and do not tighten it chasing a bug.

## Ground rules that earned their keep

- **Instrument before theorising.** Every real find came from two printfs; every
  confident theory, mine included, was wrong.
- **Beware the vacuous pass.** A "0 crashes in 16 runs" verification here was
  measuring nothing: the suite had deleted `rsmtst.a`, so r68 was assembling a
  file that did not exist. Check the run did the work before believing the
  result.
- **Check for company before trusting a flaky suite.** Both worktrees' `h0`
  symlink to the same `../os9/h0`, so a second session running tests deletes
  your fixtures mid-run. That produced a completely convincing "second bug"
  (failures clustered on `t_text`, ~1 run in 5) that did not exist. Tell:
  it would not reproduce in 2000 isolated iterations. Settle it with a private
  disk, not with more runs -- see the `shared-h0-breaks-concurrent-test-runs`
  note.
- **Never `pkill -f os9exec`** as a hang guard: it kills the test suite and the
  other session too. Match the worktree path.
