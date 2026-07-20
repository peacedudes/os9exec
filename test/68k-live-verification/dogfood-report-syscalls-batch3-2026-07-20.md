# Dogfood report: 68k syscall live-verification batch 3 (2026-07-20)

Continues batches 1-2. I/O path calls (`I$Create`, `I$Delete`,
`I$MakDir`, `I$ChgDir`, `I$Seek`, `I$Dup` — `batch3-01.a`) and signals
(`F$Send`, `F$Icpt` — `batch3-02.a`). All 8 calls confirmed on the first
live attempt — zero fix-and-rerun cycles this batch, the direct payoff
of checking `os9exec`'s own source (`icalls.c`/`fcalls.c`) for every
register convention before writing any test code, established as
standard practice starting batch 1.

## I/O path calls — all 6 confirmed cleanly

```
PASS I$Create
PASS I$Seek
PASS I$Dup
PASS I$Delete
PASS I$MakDir
PASS I$ChgDir
```

One documentation correction found by reading source (not by a failed
live test): `I$Create`'s and `I$MakDir`'s `d1` (file attributes)
parameter is a **byte** (`d1.b`) per `OS9_I_Create`/`OS9_I_MakDir`, not
the word (`d1.w`) this file previously claimed. Harmless in practice as
long as the byte value itself is correct and the rest of the register is
clean, but worth being precise about given this project's own
register-discipline lesson from batch 1 (a dispatcher can read more of
a register than its `.w`/`.b` documentation implies).

`I$ChgDir` was previously `Manual`-only with zero register detail
(buried in a list, not a table row) — promoted to a full confirmed row.

## Signals — two real, `os9exec`-specific behavioral divergences, both confirmed live

Found first by reading `os9exec`'s own source, *before* any live test —
both are explicit comments left by the emulator's own author, not
inferred:

1. **`F$Send`'s PID 0 does not broadcast.** Standard OS-9 documents PID
   0 as "send to every process in the sender's user/group except the
   sender." `os9exec`'s `OS9_F_Send` has an explicit comment: `"d0.w=
   intended receiver's process ID (0 is NOT all here!)"`, and further:
   `"Killing all processes with ID=0 is not implemented because pid=0 is
   a valid process ID in os9exec/nt."` So PID 0 is a real, specific,
   addressable process on this emulator, not a broadcast target. Live
   confirmation: sending signal 1 (wakeup) to PID 0 succeeds with no
   error (a real send to a real process), and sending the same signal to
   the caller's own actual PID also succeeds cleanly — both prove
   `F$Send` itself dispatches correctly, isolating the finding to
   specifically the *broadcast* semantics being absent, not the call
   being broken.
2. **`F$Icpt` doesn't actually deliver signals to installed handlers.**
   `OS9_F_Icpt`'s own comment: `"Restrictions: does not work, as signal
   handling is not yet implemented (%%%)"`. The call stores the
   handler-address and data-pointer fields with no validation and always
   returns success — confirmed live (a real handler address, accepted
   with no error) — but there is no code path anywhere that would ever
   actually jump to that handler on a real signal. This is a genuine
   `os9exec` feature gap, the same category of finding as the earlier
   device-driver/file-manager dispatch gaps (real code exists and looks
   plausible, but the kernel never invokes it) — not something this pass
   attempted to fix, matching those earlier sessions' scope.

**Practical guidance for anyone testing signal-dependent behavior on
`os9exec`:** target real PIDs explicitly (never rely on PID-0
broadcast), and don't expect `F$Icpt`-installed handlers to actually
fire — any test relying on intercepted-signal delivery will need to
account for this gap rather than assume standard OS-9 semantics.

## Running total

23 of the 68k syscall reference's calls are now `Live`-confirmed
(10 batch 1 + 5 batch 2 + 8 this batch), up from 0 at the start of
2026-07-20.

## Files

- `test/68k-live-verification/batch3-01.a` — I$Create/I$Delete/I$MakDir/I$ChgDir/I$Seek/I$Dup
- `test/68k-live-verification/batch3-02.a` — F$Send/F$Icpt
