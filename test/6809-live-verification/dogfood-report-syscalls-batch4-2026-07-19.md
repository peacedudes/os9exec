# Dogfood report: 6809 syscall batch 4 (2026-07-19)

Continues batches 1-3. Also fixed three long-standing row-level gaps in
`syscalls-and-module-format.md`: `F$Exit`/`F$Send`/`F$Icpt` were already
confirmed `Live` (mentioned in this file's own top-of-file header from
earlier sessions) but their individual table rows never got tagged —
fixed, no new testing needed.

Target set for this batch, chosen for genuine safety (bounded,
self-contained, no shared-system mutation) after explicitly excluding
the riskier remainder (`F$Boot`, `F$SUser`, task/process-table
primitives, DAT/memory-map primitives, module-directory-internal calls
the docs themselves flag as kernel-only): `F$Load`/`F$UnLoad`
(`batch4-01.a`), `F$SSWI` (`batch4-02.a`), `F$SRqMem`/`F$SRtMem`
(`batch4-03.a`), `F$Move`/`F$FModul`/`I$Attach`/`I$Detach`
(`batch4-04.a`).

## Real bugs found in the test sources themselves (same class as before)

Wrote these myself this time (no sub-agent involved), and hit the exact
same missing-store-before-print bug class documented in batches 1-3
anyway — worth noting it's not just an artifact of delegation, it's an
easy mistake regardless of who's writing the test:

- `batch4-03.a`'s `F$SRqMem`/`F$SRtMem` failure branches did
  `clra`/`lbsr prdec` with no `stb ERRB,u` ever having captured the
  real error code first — printed `err=51264` (impossible for an 8-bit
  register). Fixed: `stb ERRB,u` immediately after the register-juggling
  following each `swi2`, before `copys` (which reuses `B` as a loop
  counter) can clobber it.
- `batch4-04.a`'s `F$Move`/`I$Attach`/`I$Detach` failure branches had
  the identical bug. `F$FModul`'s did not (written correctly the first
  time in the same file — inconsistency within my own single pass, not
  a systematic gap).

## Live results

### `batch4-01.a` (`SysLdTst`) — `F$Load`/`F$UnLoad`

```
PASS F$Load entry=57592
FAIL F$UnLoad err=00221
```

- **`F$Load`**: confirmed. First attempt used a bare name (`hello`) that
  exists only in the disk's *root* directory (this session's current
  data directory) and failed `E$PNNF`/216 — switched to `hello4`
  (present in `CMDS`) and it succeeded immediately, returning a
  plausible entry-point address. **Real finding**: `F$Load`'s path
  resolution behaves like `F$Fork`'s (execution-directory search), not
  like `I$Open`'s (current-data-directory search) — a bare name has to
  actually be reachable via the exec-dir list, being present in the
  current data directory alone isn't enough.
- **`F$UnLoad`**: failed `E$MNF` (221, "module not found") immediately
  after successfully loading that exact name in the same process. Not
  chased further this pass — `mdir` (the system module directory
  listing utility) didn't show `hello4` *or* this test's own
  `SysLdTst` module resident either, so either `F$Load`-only modules
  don't register the same way modules `mdir` enumerates, or `A=type`
  isn't literally the type/lang byte `F$Load` returns in `A` the way
  this test assumed. Left inconclusive.

### `batch4-02.a` (`SysSwTst`) — `F$SSWI`

```
FAIL F$SSWI handler did not run
```

The install call itself (`F$SSWI`, vector 1/plain `SWI` — deliberately
*not* vector 2, since `SWI2` is this whole suite's own syscall dispatch
mechanism and clobbering it would break every subsequent call in the
same program) was accepted with no error — that half is confirmed.
Triggering a bare `SWI` afterward and checking a memory marker the
handler was supposed to set showed the handler never ran (or its exit
didn't return control the way this test expected). The handler exits
via plain `RTI`, the only documented exit convention anywhere in this
project's references — borrowed by analogy from `F$Icpt`'s row, which
explicitly says "exits via RTI." `F$SSWI`'s own row says nothing about
entry/exit convention at all. Left genuinely unconfirmed rather than
asserted as a bug — the raw-hardware-vector mechanism and the
signal-delivery mechanism may just have different, undocumented
conventions.

### `batch4-03.a` (`SysMemTst`) — `F$SRqMem`/`F$SRtMem`, after the bug fix above

```
FAIL F$SRqMem err=00208
```

`err=00208` = `E$UnkSvc` ("Illegal service request — Unknown service
code"), confirmed via `common/error-codes.md`. **This is the same error
code `F$GProcP` failed with in batch 3** (at the time left as
"uninvestigated" — now resolved: it's genuinely unimplemented on this
kernel build, not a test-code issue). `F$SRtMem` was never reached as a
result (test bails on the paired `F$SRqMem` failure).

### `batch4-04.a` (`SysMscTst`) — `F$Move`/`F$FModul`/`I$Attach`/`I$Detach`, after the bug fix above

```
FAIL F$Move err=00208
FAIL F$FModul err=00208
PASS I$Attach
PASS I$Detach
```

- **`F$Move`**, **`F$FModul`**: both `E$UnkSvc` (208) — same pattern as
  `F$GProcP`/`F$SRqMem`. Both rows already documented a Level-2-only
  dependency ("not needed on a single-address-space Level 1 system" /
  "internally calls `F$DATLog`/`F$LDAXY`/`F$LDDDXY` (all Level-2-only)")
  — this confirms it live rather than leaving it as a documentation
  guess. **Emerging pattern across batches 3-4: `err=00208` on this
  kernel build reliably means "genuinely not implemented," specifically
  for calls whose own documentation already flags a Level-2/multi-
  address-space dependency.** Worth remembering as a fast diagnostic for
  any future syscall test that fails this way — don't assume a test bug
  first.
- **`I$Attach`/`I$Detach`**: both confirmed cleanly. First attempt used
  device name `/T1`, which doesn't exist on this system (this REPL's
  shell prompt itself, `{N1|NN}/DD:`, names the actual channel device —
  `/N1`) — switched to `/N1` and both calls succeeded immediately.
  Since `/N1` was already attached and in use by the REPL's own
  connection, `I$Detach` correctly just decremented a use count rather
  than tearing the device down (confirmed by the REPL channel
  continuing to work normally afterward).

## Harness notes

Same login/ToolShed recipe as the fixed batch 3 (`login USER1` first,
`asm CLAUDE/<file>.a -O=<name>` to actually persist a `CMDS` module).
No new harness gotchas this batch beyond what's already documented.

## Files

- `test/6809-live-verification/batch4-01.a` — `F$Load`/`F$UnLoad`
- `test/6809-live-verification/batch4-02.a` — `F$SSWI`
- `test/6809-live-verification/batch4-03.a` — `F$SRqMem`/`F$SRtMem` (1 bug fixed)
- `test/6809-live-verification/batch4-04.a` — `F$Move`/`F$FModul`/`I$Attach`/`I$Detach` (1 bug fixed)
