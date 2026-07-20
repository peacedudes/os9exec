# Dogfood report: 68k syscall live-verification batch 10 (2026-07-20)

Continues batch 9. `F$SRqMem`, `F$SRtMem`, `F$SRqCMem`, `F$CpyMem` —
`batch10-01.a`.

## Confirmed cleanly

- **`F$SRqMem`/`F$SRtMem`**: requested 256 bytes, wrote a recognizable
  byte pattern through the *returned pointer* and read it back before
  trusting the "success" — confirms it's real, usable, writable memory,
  not just a success code with nothing backing it. Returned the block
  via `F$SRtMem` using the exact size `F$SRqMem` reported (its own
  documented restriction: a mismatched size silently doesn't free it).
- **`F$SRqCMem`**: `funcdispatch.c` shows this shares `OS9_F_SRqMem`'s
  own C body — the "color" parameter in `d1.l` is declared in the
  dispatch table's input mask but the shared handler never reads `d1`
  at all. Confirmed live: a deliberately nonzero color (`$12345678`)
  had no effect, behaved identically to a plain request.
- **`F$CpyMem`**: copied a known 16-byte pattern between two local
  buffers, confirmed byte-for-byte.

## A real `os9exec` finding, logged on `ROADMAP.md`: no ownership check

`OS9_F_CpyMem`'s doc comment says `d0.w` is "process ID of external
memory's owner" — but the C body never reads `rp->d[0]` anywhere. The
call is a raw address-range-checked `memcpy`, with **no verification
that the source address actually belongs to the process named in
`d0.w`**. Any process could read/write another process's arena memory
just by knowing (or guessing) an address, regardless of the PID
parameter. This test's own PID value is cosmetic — the copy would have
worked identically with any value there. Logged as a real
permission-model gap on `ROADMAP.md`, same family as the RBF
permission-enforcement work already done for this project, just never
extended to this call.

## Running total

45 of the 68k syscall reference's calls are now `Live`-confirmed (41 +
4 this batch).

## Files

- `test/68k-live-verification/batch10-01.a`
