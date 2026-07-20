# Dogfood report: 68k syscall live-verification batch 5 (2026-07-20)

Continues batches 1-4. `F$SigMask`, `F$DatMod`, `F$Alarm` —
`batch5-01.a`. All three confirmed on the first live attempt.

Caught one real mistake before deploying, not after: this file's first
draft hardcoded `F$Alarm`'s dispatch code as `$1B` from memory without
checking — a quick look at `funcdispatch.c` showed the real code is
`$56` (`$1B` is actually `F$CpyMem`). Fixed before ever running it.
Worth noting since it's exactly the kind of mistake the source-first
methodology is supposed to prevent, caught this time only because of a
deliberate "verify the constant, don't trust memory" habit rather than
the methodology being foolproof.

`F$Alarm`'s subfunction codes (`A$Delete`=0, `A$Set`=1, `A$Cycle`=2,
`A$AtDate`=3, `A$AtJul`=4) were previously undocumented as actual
numbers (the skill only named them) — confirmed via `alarms.c`'s
`Alarm()` dispatch switch and live-tested (`A$Set` with a far-future
interval that will never fire, immediately followed by `A$Delete` on
the returned ID — a clean set/delete round trip without needing to
handle real signal delivery).

## Deferred to a separate batch: F$DFork/F$DExec/F$DExit

Read all three (`Source/OS9exec_core/fcalls.c`) — genuinely testable
(fork a suspended debug child, single-step or run it, kill it), a
different shape of test than anything run so far (needs a real
register-frame buffer for `F$DFork`'s `(a2)` input, and `F$DExec` parks
the calling process until the child stops). Worth its own focused
pass rather than folding in here.

## Skipped, confirmed too risky to call automated

- **`F$SysDbg`**: source shows it calls `debugwait()` — `os9exec`'s own
  *interactive* meta-debugger prompt, a different, lower-level debugger
  than the guest OS-9 `debug` command already tested elsewhere. The
  scripted REPL has no way to interact with that prompt; calling this
  would hang the session. Same risk category as `F$Boot` on the 6809
  side.
- **`F$RTE`**: source's own comment says it kills the process if called
  when not actually inside an intercept routine (`procs[].lastsignal==0`).
  Since `F$Icpt` doesn't deliver signals at all (confirmed batch 3),
  there's no way to legitimately reach a real intercept context to test
  this safely — calling it directly would just kill the test process.
- **`F$TLink`**: needs a real trap-handler (`TrapLib`-type) module to
  link against, which doesn't exist yet in this test suite. Deferred,
  not skipped for risk reasons — just needs more setup than this batch
  had time for.

## Running total

30 of the 68k syscall reference's calls are now `Live`-confirmed
(27 after batch 4 + 3 this batch).

## Files

- `test/68k-live-verification/batch5-01.a` — F$SigMask/F$DatMod/F$Alarm
